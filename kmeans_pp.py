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
    datapoints1 = pd.read_csv(file_name_1, sep = ",")
    datapoints2 = pd.read_csv(file_name_2, sep = ",")
    datapoints1 = datapoints1.set_index(datapoints1.columns[0])
    datapoints2 = datapoints2.set_index(datapoints2.columns[0])
    merged_df = pd.merge(datapoints1, datapoints2,  left_index=True, right_index=True)
    merged_df.sort_index(inplace=True)
    lst = merged_df.values.tolist()
    return lst
    
    return merged_df

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
    print("helloooo")
    a = load_datapoints(r"C:\Users\Altar\Downloads\tests\tests\input_1_db_1.txt", r"C:\Users\Altar\Downloads\tests\tests\input_1_db_2.txt")
    print(a)
    # datapoints1 = pd.read_csv(r"C:\Users\Altar\Downloads\tests\tests\input_1_db_1.txt")
    # datapoints1 = datapoints1.set_index(datapoints1.columns[0])
    # print(datapoints1)
    # print("dp2")
    # datapoints2 = pd.read_csv(r"C:\Users\Altar\Downloads\tests\tests\input_1_db_2.txt")
    # datapoints2 = datapoints2.set_index(datapoints2.columns[0])
    # datapoints2.sort_values(datapoints2.columns[0])
    # print(datapoints2)
    # print(pd.read_csv(r"C:\Users\Altar\Downloads\tests\tests\input_1_db_2.txt"))


if __name__ == "__main__":
    main()