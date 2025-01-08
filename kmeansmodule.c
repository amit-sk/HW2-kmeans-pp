#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

const int SUCCESS = 0;
const int ERROR = 1;

const double EPSILON = 0.001;
const int DEFAULT_ITER = 200;

const char *GENERIC_ERROR_MSG = "An Error Has Occurred\n";
const char *INVALID_K_ERROR_MSG = "Invalid number of clusters!\n";
const char *INVALID_ITER_ERROR_MSG = "Invalid maximum iteration!\n";

struct centroid {
    struct coord *centroid_coords;
    struct coord *sum;
    int count;
};

struct coord
{
    double coord;
    struct coord *next;
};

struct datapoint
{
    struct coord *coords;
    struct datapoint *next;
};

static PyObject* fit(PyObject *self, PyObject *args);

static PyMethodDef kmeansMethods[] = {
    "fit",
    (PyCFunction)fit,
    METH_VARARGS,
    PyDoc_STR("TODO - docstring"),
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef kmeansmodule = {
    PyModuleDef_HEAD_INIT,
    "kmeans",
    NULL,
    -1,
    kmeansMethods
};

PyMODINIT_FUNC PyInit_kmeans(void) {
    PyObject *m;
    m = PyModule_Create(&kmeansmodule);
    if (!m) {
        return NULL;
    }
    return m;
}

static PyObject* fit(PyObject *self, PyObject *args) {
    return Py_BuildValue("d", 0.0); 
}

void add_coord_to_centroid(struct centroid *cent, struct datapoint *point, int d){
    int i;
    struct coord *curr_sum_coord = cent->sum;
    struct coord *curr_datapoint_coord = point->coords;

    for (i = 0; i < d; i++){
        curr_sum_coord->coord += curr_datapoint_coord->coord;
        curr_sum_coord = curr_sum_coord->next;
        curr_datapoint_coord = curr_datapoint_coord->next;
    }
}

double calc_euclidean_distance(struct coord *coord1, struct coord *coord2, int d){
    double sum = 0;
    int i;

    for (i = 0; i < d; i++){
        sum += pow((coord1->coord - coord2->coord), 2);
        coord1 = coord1->next;
        coord2 = coord2->next;
    }
    return sqrt(sum);
}


void run_kmeans(int d, int K, int iter, struct datapoint *points, struct centroid *centroids) {
    int is_not_converged = 1;
    int j = 0;
    int k = 0;
    struct datapoint *point = NULL;
    struct centroid *cent = NULL;
    int index = 0;
    int i = 0;
    
    for (i = 0; i < iter && is_not_converged; i++) {
        /* for each point, find closest centroid and add to its sum. */
        for (point = points; point != NULL; point = point->next) {
            struct centroid *min_cent = centroids;
            double min_distance = HUGE_VAL;
            double curr_distance;

            for (index = 0; index < K; index++){
                curr_distance = calc_euclidean_distance((centroids + index)->centroid_coords, point->coords, d);
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
            struct coord *curr_coord = cent->centroid_coords;
            struct coord *sum_coord = cent->sum;
            struct coord *curr_sum_coord = sum_coord;
            int count = cent->count;

            for (k = 0; k < d; k++) {
                if (count != 0) {
                    curr_sum_coord->coord = curr_sum_coord->coord/count;
                }
                curr_sum_coord = curr_sum_coord->next;
            }

            if (calc_euclidean_distance(cent->centroid_coords, sum_coord, d) >= EPSILON) {
                is_not_converged = 1;
            }

            /* reset sum and count */
            for (k = 0; k < d; k++) {
                curr_coord->coord = sum_coord->coord;
                sum_coord->coord = 0;
                curr_coord = curr_coord->next;
                sum_coord = sum_coord->next;
            }
            cent->count = 0;
        }
    }
}

void free_coords(struct coord *coord) {
    struct coord *curr_coord = coord;
    while (curr_coord != NULL) {
        struct coord *next_coord = curr_coord->next;
        free(curr_coord);
        curr_coord = next_coord;
    }
}

void free_all(int K, struct datapoint *datapoints, struct centroid *centroids) {
    int i;
    struct centroid *curr_centroid;
    struct datapoint *curr_datapoint;
    struct datapoint *next_datapoint;

    curr_datapoint = datapoints;
    while (curr_datapoint != NULL) {
        next_datapoint = curr_datapoint->next;
        free_coords(curr_datapoint->coords);
        free(curr_datapoint);
        curr_datapoint = next_datapoint;
    }

    if (centroids != NULL) {
        for (curr_centroid = centroids, i = 0; i < K; i++, curr_centroid++) {
            free_coords(curr_centroid->centroid_coords);
            free_coords(curr_centroid->sum);
        }
        free(centroids);
    }
}

void print_results(int K, struct centroid *centroids) {
    int i = 0;
    for (; i < K; i++){
        struct coord *curr_coord = (centroids + i)->centroid_coords;
        do {
            printf("%.4f", curr_coord->coord);
            curr_coord = curr_coord->next;
            if (curr_coord != NULL) {
                printf(",");
            }
        } while (curr_coord != NULL);
        printf("\n");
    }
}
