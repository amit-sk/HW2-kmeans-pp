import numpy as np
import pandas as pd
import sys

DEFAULT_ITER = 300

GENERIC_ERROR_MSG = "An Error Has Occurred"
INVALID_K_ERROR_MSG = "Invalid number of clusters!"
INVALID_EPS_ERROR_MSG = "Invalid epsilon!"
INVALID_ITER_ERROR_MSG = "Invalid maximum iteration!"

def is_integer(string):
    try:
        return float(string) == int(float(string))
    except ValueError:
        return False

def load_datapoints(file_name_1, file_name_2):
    datapoints1 = pd.read_csv(file_name_1, sep = ",", header=None)
    datapoints2 = pd.read_csv(file_name_2, sep = ",", header=None)
    datapoints1 = datapoints1.set_index(datapoints1.columns[0])
    datapoints2 = datapoints2.set_index(datapoints2.columns[0])
    merged_df = pd.merge(datapoints1, datapoints2,  left_index=True, right_index=True)
    merged_df.sort_index(inplace=True)
    lst = merged_df.values.tolist()
    indexes = [int(ind) for ind in merged_df.index.to_list()]
    return lst, indexes

def read_args():
    argc = len(sys.argv)
    if argc < 5 or argc > 6:
        print(GENERIC_ERROR_MSG)
        return 0, 0, [], [], 0, 0, 0, False
    
    # get & check iter
    iter = DEFAULT_ITER
    if len(sys.argv) == 6:
        iter = int(float(sys.argv[2])) if is_integer(sys.argv[2]) else None

    if iter is None or (not 1 < iter < 1000):
        print(INVALID_ITER_ERROR_MSG)
        return 0, 0, [], [], 0, 0, 0, False
    
    # get K
    K = int(float(sys.argv[1])) if is_integer(sys.argv[1]) else None
    
    # get & check files
    file_name_1, file_name_2 = sys.argv[-2:]
    datapoints, datapoint_indexes = load_datapoints(file_name_1, file_name_2)
    # get N
    N = len(datapoints)

    # check K
    if K is None or (not 1 < K < N):
        print(INVALID_K_ERROR_MSG)
        return 0, 0, [], [], 0, 0, 0, False
    
    # get eps
    try:
        eps = float(sys.argv[-3])
    except ValueError:
        print(INVALID_EPS_ERROR_MSG)
        return 0, 0, [], [], 0, 0, 0, False
    
    # get d
    d = len(datapoints[0])
    
    return K, iter, eps, datapoints, datapoint_indexes, N, d, True

def euclidean_distance(point1, point2):
    return (sum([(point1[i] - point2[i])**2 for i in range(len(point1))]))**0.5

def init_centroids(datapoints, datapoint_indexes, K):
    not_chosen = set(datapoint_indexes)
    first_centroid_ind = np.random.choice(datapoint_indexes)
    centroids = [datapoints[first_centroid_ind]]
    centroid_indexes = [first_centroid_ind]
    not_chosen.remove(first_centroid_ind)

    for i in range(K-1):
        not_chosen_ls = list(not_chosen)
        D_values = [min([euclidean_distance(datapoints[point], centroid) for centroid in centroids]) for point in not_chosen_ls]
        D_sum = sum(D_values)
        P_values = [D_value/D_sum for D_value in D_values]

        new_centroid_ind = np.random.choice(not_chosen_ls, p=P_values)
        centroids.append(datapoints[new_centroid_ind])
        centroid_indexes.append(new_centroid_ind)
        not_chosen.remove(new_centroid_ind)

    return centroids, centroid_indexes

# TODO: remove this func, only for tests
def run_kmeans(K, datapoints, centroids, d, eps, iter):
    for i in range(iter):
        old_centroids = centroids
        centroids_sums = [[0 for i in range(d)] for j in range(K)]
        centroids_counters = [0 for i in range(K)]
        
        for point in datapoints:
            min_dist_centroid = 0
            min_dist = euclidean_distance(point, centroids[0])

            for ind, centroid in enumerate(centroids):
                dist = euclidean_distance(point, centroid)

                # if the point is closest to this centroid so far:
                if dist < min_dist:
                    # update the info of the closest centroid
                    min_dist = dist
                    min_dist_centroid = ind
            
            # add the point to the closest centroid's sum and up its counter by 1
            for coord in range(d):
                centroids_sums[min_dist_centroid][coord] += point[coord]
            centroids_counters[min_dist_centroid] += 1
        
        # set the new centroids to the mean of all of the points closest to them
        centroids = [[centroids_sums[j][i] / centroids_counters[j] for i in range(d)] for j in range(K)]

        # if the centroids barely moved since the last iteration, stop
        if max([euclidean_distance(centroids[i], old_centroids[i]) for i in range(K)]) < eps:
            break
    
    return centroids

def main():
    np.random.seed(1234)

    K, iter, eps, datapoints, datapoint_indexes, N, d, success = read_args()
    if not success:
        return
    
    centroids, centroid_indexes = init_centroids(datapoints, datapoint_indexes, K)
    centroids = run_kmeans(K, datapoints, centroids, d, eps, iter)

    # centroid indexes
    print(",".join([str(i) for i in centroid_indexes]))
    # final centroid values
    print("\n".join([",".join(["%.4f" % coord for coord in centroid]) for centroid in centroids]))

if __name__ == "__main__":
    try:
        main()
    except Exception:
        print(GENERIC_ERROR_MSG)