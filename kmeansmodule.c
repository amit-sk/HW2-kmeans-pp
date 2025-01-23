#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define GOTO_CLEANUP_IF_NULL(x) { if (x == NULL) { goto cleanup; } }
#define GOTO_CLEANUP_IF_NEGATIVE(x) { if (x < 0) { goto cleanup; } }
#define GOTO_CLEANUP_IF_PYERROR_OCCURED() { if (NULL != PyErr_Occurred()) { goto cleanup; } }

const int SUCCESS = 0;
const int ERROR = 1;

const char *GENERIC_ERROR_MSG = "An Error Has Occurred\n";
const char *INVALID_K_ERROR_MSG = "Invalid number of clusters!\n";
const char *INVALID_ITER_ERROR_MSG = "Invalid maximum iteration!\n";

struct centroid {
    double *centroid_coords;
    double *sum;
    int count;
};

static PyObject* fit(PyObject *self, PyObject *args);

static PyMethodDef kmeansMethods[] = {
    "fit",
    (PyCFunction)fit,
    METH_VARARGS,
    PyDoc_STR("fit method for K-means clustering\nargs:\n initial_centroids: A Python list of K initialized centroids, each being a list of D floats.\n datapoints: A Python list of N data points, each being a list of D floats.\n max_iter (int): Maximum number of iterations to run the K-means algorithm.\n epsilon (double): Convergence threshold (for centroid change)."),
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef kmeansmodule = {
    PyModuleDef_HEAD_INIT,
    "mykmeanssp",
    NULL,
    -1,
    kmeansMethods
};

PyMODINIT_FUNC PyInit_mykmeanssp(void) {
    PyObject *m;
    m = PyModule_Create(&kmeansmodule);
    if (!m) {
        return NULL;
    }
    return m;
}

/*  
  Fit method arguments:
   initial_centroids (PyObject*): A Python list of K initialized centroids.
   datapoints (PyObject*): A Python list of N data points, each being a list of D floats.
   max_iter (int): Maximum number of iterations to run the K-means algorithm.
   epsilon (double): Convergence threshold (for centroid change).
  
  Returns:
   PyObject*: A Python list of the final centroids, each being a list of D floats.
 */
static PyObject* fit(PyObject *self, PyObject *args) {
    PyObject *centroid_list = NULL;  /* output value */
    PyObject *centroids = NULL;
    PyObject *datapoints = NULL;
    unsigned int iter = 0;
    double eps = 0.0;
    PyObject *curr_item = NULL;
    PyObject *curr_coor = NULL;
    double *datapoints_array = NULL;
    struct centroid *centroids_array = NULL;

    if (!PyArg_ParseTuple(args, "OOId", &centroids, &datapoints, &iter, &eps)) {
        goto cleanup;
    }

    int N = PyObject_Length(datapoints);
    GOTO_CLEANUP_IF_NEGATIVE(N);

    curr_item = PyList_GetItem(datapoints, 0);
    GOTO_CLEANUP_IF_NULL(curr_item);
    int d = PyObject_Length(curr_item);
    GOTO_CLEANUP_IF_NEGATIVE(d);

    datapoints_array = (double *)calloc(N*d, sizeof(double));
    GOTO_CLEANUP_IF_NULL(datapoints_array);

    for (int i = 0; i<N; i++) {
        curr_item = PyList_GetItem(datapoints, i);
        GOTO_CLEANUP_IF_NULL(curr_item);
        for (int j = 0; j<d; j++) {
            curr_coor = PyList_GetItem(curr_item, j);
            GOTO_CLEANUP_IF_NULL(curr_coor);
            datapoints_array[i*d+j] = PyFloat_AsDouble(curr_coor);
            GOTO_CLEANUP_IF_PYERROR_OCCURED();
        }
    }

    int K = PyObject_Length(centroids);
    GOTO_CLEANUP_IF_NEGATIVE(K);

    centroids_array = (struct centroid *)calloc(K, sizeof(struct centroid));
    GOTO_CLEANUP_IF_NULL(centroids_array);

    for (int i = 0; i<K; i++) {
        curr_item = PyList_GetItem(centroids, i);
        GOTO_CLEANUP_IF_NULL(curr_item);

        centroids_array[i].centroid_coords = (double *)calloc(d, sizeof(double));
        GOTO_CLEANUP_IF_NULL(centroids_array[i].centroid_coords);
        centroids_array[i].sum = (double *)calloc(d, sizeof(double));
        GOTO_CLEANUP_IF_NULL(centroids_array[i].sum);

        for (int j = 0; j<d; j++){
            curr_coor = PyList_GetItem(curr_item, j);
            GOTO_CLEANUP_IF_NULL(curr_coor);
            centroids_array[i].centroid_coords[j] = PyFloat_AsDouble(curr_coor);
            GOTO_CLEANUP_IF_PYERROR_OCCURED();
        }
    }

    run_kmeans(N, d, K, iter, eps, datapoints_array, centroids_array);

    centroid_list = PyList_New(K);
    GOTO_CLEANUP_IF_NULL(centroid_list);

    for (int i = 0; i<K; i++){
        PyObject *centroid_coordinates = PyList_New(d);
        GOTO_CLEANUP_IF_NULL(centroid_coordinates);
        for (int j = 0; j<d; j++){
            curr_coor = PyFloat_FromDouble(centroids_array[i].centroid_coords[j]);
            GOTO_CLEANUP_IF_NULL(curr_coor);
            GOTO_CLEANUP_IF_NEGATIVE(PyList_SetItem(centroid_coordinates, j, curr_coor));
        }
        GOTO_CLEANUP_IF_NEGATIVE(PyList_SetItem(centroid_list, i, centroid_coordinates));
    }

cleanup:
    free_all(K, datapoints_array, centroids_array);

    return centroid_list; 
}

void add_coord_to_centroid(struct centroid *cent, double *point_coords, int d){
    int i;
    double *sum_coords = cent->sum;

    for (i = 0; i < d; i++){
        sum_coords[i] += point_coords[i];
    }
}

double calc_euclidean_distance(double *coord1, double *coord2, int d){
    double sum = 0;
    int i;

    for (i = 0; i < d; i++){
        sum += pow((coord1[i] - coord2[i]), 2);
    }
    return sqrt(sum);
}


void run_kmeans(int N, int d, int K, int iter, double eps, double *points, struct centroid *centroids) {
    int is_not_converged = 1;
    int j = 0;
    int k = 0;
    double *point = NULL;
    struct centroid *cent = NULL;
    int index = 0;
    int i = 0;
    
    for (i = 0; i < iter && is_not_converged; i++) {
        /* for each point, find closest centroid and add to its sum. */
        for (j = 0, point = points; j < N; j++, point += d) {
            struct centroid *min_cent = centroids;
            double min_distance = HUGE_VAL;
            double curr_distance;

            for (index = 0; index < K; index++){
                curr_distance = calc_euclidean_distance((centroids + index)->centroid_coords, point, d);
                if (curr_distance < min_distance){
                    min_distance = curr_distance;
                    min_cent = centroids + index;
                }
            }
            min_cent->count++;
            add_coord_to_centroid(min_cent, point, d);
        }
 
        /* update centroids and check convergance. */
        cent = centroids;
        is_not_converged = 0;
        for (j = 0; j < K; j++, cent++) {
            double *cent_coords = cent->centroid_coords;
            double *sum_coords = cent->sum;
            int count = cent->count;

            for (k = 0; k < d; k++) {
                if (count != 0) {
                    sum_coords[k] = sum_coords[k]/count;
                }
            }

            if (calc_euclidean_distance(cent_coords, sum_coords, d) >= eps) {
                is_not_converged = 1;
            }

            /* reset sum and count */
            for (k = 0; k < d; k++) {
                cent_coords[k] = sum_coords[k];
                sum_coords[k] = 0;
            }
            cent->count = 0;
        }
    }
}

void free_all(int K, struct datapoint *datapoints, struct centroid *centroids) {
    int i;
    struct centroid *curr_centroid;

    free(datapoints);

    if (centroids != NULL) {
        for (curr_centroid = centroids, i = 0; i < K; i++, curr_centroid++) {
            free(curr_centroid->centroid_coords);
            free(curr_centroid->sum);
        }
        free(centroids);
    }
}
