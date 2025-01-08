#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

/* internal funcs for read_args */
int init_datapoint(struct datapoint **datapoint, struct coord *first_coord) {
    struct datapoint *new_datapoint = malloc(sizeof(struct datapoint));
    if (new_datapoint == NULL) {
        return ERROR;
    }
    new_datapoint->coords = first_coord;
    new_datapoint->next = NULL;

    *datapoint = new_datapoint;
    return SUCCESS;    
}

int init_coord(struct coord **coord, double n) {
    struct coord *new_coord = malloc(sizeof(struct coord));
    if (new_coord == NULL) {
        return ERROR;
    }
    new_coord->coord = n;
    new_coord->next = NULL;

    *coord = new_coord;
    return SUCCESS;
}

int parse_file(int *d, int *N, struct datapoint **datapoints) {
    double n; /* for the double values */
    char delim; /* commas, \n, ... */
    struct datapoint **curr_datapoint = datapoints;
    struct coord *first_coord = NULL;
    struct coord **curr_coord = NULL;

    /* go over first line to get d */
    first_coord = NULL;
    curr_coord = &first_coord;
    do {
        scanf("%lf%c", &n, &delim);
        if (SUCCESS != init_coord(curr_coord, n)) {
            return ERROR;
        }
        curr_coord = &(*curr_coord)->next;
        (*d)++;
    } while (delim != '\n');

    /* initialize the first datapoint */
    if (SUCCESS != init_datapoint(curr_datapoint, first_coord)) {
        return ERROR;
    }
    curr_datapoint = &(*curr_datapoint)->next;
    (*N)++;

    /* go over the rest of the lines to get datapoints and N */
    first_coord = NULL;
    curr_coord = &first_coord;
    while (scanf("%lf%c", &n, &delim) == 2) {
        if (SUCCESS != init_coord(curr_coord, n)) {
            return ERROR;
        }
        curr_coord = &(*curr_coord)->next;

        if (delim == '\n') { /* if at the end of the line */
            if (SUCCESS != init_datapoint(curr_datapoint, first_coord)) {
                return ERROR;
            }
            curr_datapoint = &(*curr_datapoint)->next;
            first_coord = NULL;
            curr_coord = &first_coord;
            (*N)++;
        }
    }

    return SUCCESS;
}

int parse_integer(char *src, int *dest) {
    int n = 0;
    int chars_read = 0;
    int amount_parsed = 0;

    amount_parsed = sscanf(src, "%d%n", &n, &chars_read);
    if (amount_parsed != 1 || src[chars_read] != '\0') {
        /* failed to parse integer, or trailing characters found */
        if (src[chars_read] == '.') {
            char *trail_string = src + (chars_read + 1);
            int trail = 0;
            int trail_chars_read = 0;
            amount_parsed = sscanf(trail_string, "%d%n", &trail, &trail_chars_read);
            if (!trail && amount_parsed <= 1 && !trail_string[trail_chars_read]) {
                *dest = n;
                return SUCCESS;
            }
        }
        return ERROR;
    }

    *dest = n;
    return SUCCESS;
}

/* parse the args (K, iter and the datapoints) */
int read_args(int argc, char *argv[], int *K, int *iter, int *d, int *N, struct datapoint **datapoints) {
    /* Read arguments - argc should be 2 if there is not iter arg, 3 if there is */
    if (argc < 2 || argc > 3) {
        printf("%s", GENERIC_ERROR_MSG);
        return ERROR;
    }

    /* read and validate iter, if given */
    if (argc == 3) {
        if (SUCCESS != parse_integer(argv[2], iter) || *iter <= 1 || *iter >= 1000) {
            printf("%s", INVALID_ITER_ERROR_MSG);
            return ERROR;
        }
    }

    /* read and validate K from below */
    if (SUCCESS != parse_integer(argv[1], K) || *K <= 1) {
        printf("%s", INVALID_K_ERROR_MSG);
        return ERROR;
    }

    /* parses datapoints from file and obtains d and N */
    if (SUCCESS != parse_file(d, N, datapoints)) {
        printf("%s", GENERIC_ERROR_MSG);
        return ERROR;
    }

    /* validate K from above */
    if (*K >= *N) {
        printf("%s", INVALID_K_ERROR_MSG);
        return ERROR;
    }

    return SUCCESS;
}


int init_centroids(int d, int K, struct datapoint *points, struct centroid **centroids) {
    int i = 0;
    int j = 0;
    struct datapoint *curr_datapoint = NULL;
    struct coord **curr_coord = NULL;
    struct coord *point_coord = NULL;

    /* memory initialized as zeroes. */
    struct centroid *cent = calloc(K, sizeof(struct centroid));
    if (cent == NULL) {
        return ERROR;
    }

    /* set first K centroids to first K datapoints. */
    for (curr_datapoint = points; i < K; i++, curr_datapoint = curr_datapoint->next) {
        /* set centroid coords to datapoint coordinates */
        curr_coord = &(cent + i)->centroid_coords;
        for (point_coord = curr_datapoint->coords;
             point_coord != NULL;
             point_coord = point_coord->next, curr_coord = &(*curr_coord)->next) {
            if (SUCCESS != init_coord(curr_coord, point_coord->coord)) {
                return ERROR;
            }
        }
        
        /* set sums to zeroes on all dimensions */
        curr_coord = &(cent + i)->sum;
        for (j = 0; j < d; j++, curr_coord = &(*curr_coord)->next) {
            if (SUCCESS != init_coord(curr_coord, 0)) {
                return ERROR;
            }
        }
    }

    *centroids = cent;
    return SUCCESS;
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

int main(int argc, char *argv[]) {
    int return_code = SUCCESS;
    int K = 0;
    int iter = DEFAULT_ITER;
    int d = 0;
    int N = 0;
    struct datapoint *datapoints = NULL;
    struct centroid *centroids = NULL;

    if (SUCCESS != read_args(argc, argv, &K, &iter, &d, &N, &datapoints)) {
        return_code = ERROR;
        goto cleanup;
    }

    if (SUCCESS != init_centroids(d, K, datapoints, &centroids)) {
        printf("%s", GENERIC_ERROR_MSG);
        return_code = ERROR;
        goto cleanup;
    }

    run_kmeans(d, K, iter, datapoints, centroids);
    print_results(K, centroids);

cleanup:
    free_all(K, datapoints, centroids);
    return return_code;
}
