import matplotlib.pyplot as plt
import seaborn
seaborn.set()

def make_heatmap(title, adjacency_list, fname):
    """
    Make a heatmap of the nxn matrix to make it easier to check the
    results
    """

    # Use this label order to keep the models together
    label_order = adjacency_list.index.get_level_values(0).unique()

    adjacency_matrix = adjacency_list.unstack()
    adjacency_matrix = adjacency_matrix.reindex(
        index=label_order, columns=label_order.copy())
    adjacency_matrix.index.name = 'Source Model'
    adjacency_matrix.columns.name = 'Target Model'

    fig, ax = plt.subplots(figsize=(10, 10))
    ax.set_title(title)
    heatmap = seaborn.heatmap(
        adjacency_matrix,
        xticklabels=True, 
        yticklabels=True,
        cmap='plasma',
        square=True,
        ax=ax,
        cbar_kws={'label': 'Dissimilarity'})
    fig.tight_layout()
    fig.savefig(fname)
