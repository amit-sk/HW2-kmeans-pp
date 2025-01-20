import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import load_iris
from sklearn.cluster import KMeans
from kmeans_pp import euclidean_distance

def get_inertia(datapoints, centroids, labels):
    return sum([euclidean_distance(datapoint, centroids[labels[i]]) for i, datapoint in enumerate(datapoints)])

def plot_graph(k_range, inertia_ls, opt_k):
    opt_k_point = np.array((opt_k, inertia_ls[opt_k-1]))
    annotation_shift = np.array((0.3, 7))

    plt.cla()

    plt.title("Elbow Method for Selection of Optimal \"K\" Clusters")
    plt.xlabel("k")
    plt.ylabel("Inertia")

    plt.plot(list(k_range), inertia_ls)

    plt.scatter(opt_k_point[0], opt_k_point[1], edgecolors="k", facecolors="none", s=200)
    plt.annotate("Elbow Point", opt_k_point, opt_k_point+annotation_shift)

    plt.savefig("elbow.png")

def main():
    datapoints = load_iris()["data"]
    k_range = range(1,11)

    inertia_ls = []
    for k in k_range:
        kmeans = KMeans(n_clusters=k, init="k-means++", random_state=0)
        kmeans.fit(datapoints)
        inertia_ls.append(get_inertia(datapoints, kmeans.cluster_centers_, kmeans.labels_))
    
    plot_graph(k_range, inertia_ls, 2)

if __name__ == "__main__":
    main()