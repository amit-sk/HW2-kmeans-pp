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
    # TODO: read coordinates and join
    pass

def read_args():
    argc = len(sys.argv)
    if argc < 5 or argc > 6:
        print(GENERIC_ERROR_MSG)
        return 0, 0, [], 0, 0, 0, False
    
    iter = DEFAULT_ITER
    if len(sys.argv) == 6:
        iter = int(float(sys.argv[2])) if is_integer(sys.argv[2]) else None

    if iter is None or (not 1 < iter < 1000):
        print(INVALID_ITER_ERROR_MSG)
        return 0, 0, [], 0, 0, 0, False
    
    K = int(float(sys.argv[1])) if is_integer(sys.argv[1]) else None
    
    try:
        eps = float(sys.argv[-3])
    except ValueError:
        print(INVALID_EPS_ERROR_MSG)
        return 0, 0, [], 0, 0, 0, False

    file_name_1, file_name_2 = sys.argv[-2:]
    datapoints = load_datapoints(file_name_1, file_name_2)
    N = len(datapoints)

    if K is None or (not 1 < K < N):
        print(INVALID_K_ERROR_MSG)
        return 0, 0, [], 0, 0, 0, False
    
    d = len(datapoints[0])
    
    return iter, K, datapoints, N, d, eps, True

def main():
    np.random.seed(1234)

    iter, K, datapoints, N, d, eps, success = read_args()

if __name__ == "__main__":
    main()