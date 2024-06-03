#! env python3

from networkx.drawing.nx_pydot import graphviz_layout
import matplotlib.pyplot as plt
import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout
import numpy as np


def create_directed_graph(edges, node_labels, output_file):
    G = nx.DiGraph()
    G.add_edges_from(edges)
    pos = graphviz_layout(G, prog="twopi")
    plt.figure(figsize=(40, 40))
    nx.draw(G, pos, with_labels=False, arrows=True, node_size=0, edge_color='white')

    bbox_props = dict(boxstyle="round,pad=0.5", alpha=0.5)
    text_positions = {}

    for node, (x, y) in pos.items():
        num_labels, str_labels = node_labels[node].split('@')
        full_text = f"{num_labels}\n{node}\n{str_labels}"
        if num_labels == '0 1 0':
            box_color = 'lightgreen'
        elif num_labels == '1 0 0':
            box_color = 'orange'
        else:
            box_color = 'lightblue'
        text_obj = plt.text(x, y, full_text, fontsize=12, ha='center', va='center',
                            bbox=dict(color=box_color, **bbox_props))
        renderer = plt.gcf().canvas.get_renderer()
        bbox = text_obj.get_window_extent(renderer=renderer)
        inv = plt.gca().transData.inverted()
        bbox_data = bbox.transformed(inv)
        text_positions[node] = bbox_data

    for (start, end) in edges:
        start_bbox = text_positions[start]
        end_bbox = text_positions[end]
        start_center = start_bbox.get_points().mean(axis=0)
        end_center = end_bbox.get_points().mean(axis=0)
        direction = end_center - start_center
        norm_direction = direction / np.linalg.norm(direction)
        start_edge = start_center + norm_direction * (start_bbox.width / 2)
        end_edge = end_center - norm_direction * (end_bbox.width / 2)
        plt.annotate("",
                     xy=(end_edge[0], end_edge[1]), xycoords='data',
                     xytext=(start_edge[0], start_edge[1]), textcoords='data',
                     arrowprops=dict(arrowstyle="<|-", lw=2, color='b',
                                     shrinkA=15, shrinkB=15,
                                     connectionstyle="arc3,rad=0.2"))
    plt.savefig(output_file, format='PNG')
    plt.close()


def get_data_from_file(file_path) -> tuple[list[tuple], dict[str, list[str]]]:
    def custom_sort(data: str) -> int:
        if '*' in data:
            return 0
        return 1

    edges = []
    node_labels = {}
    current_node = None

    with open(file_path, "r") as file:
        for line in file:
            line = line.strip()
            if line == "NODE":
                if current_node:
                    lables = current_node[2:]
                    node_labels[current_node[0]] = [lable for lable in lables]
                    if len(current_node) > 2:
                        for other_node in current_node[1].split():
                            edges.append((other_node, current_node[0]))
                current_node = []
            else:
                current_node.append(line)

    if current_node:
        lables = current_node[2:]

        node_labels[current_node[0]] = [lable for lable in lables]
        if len(current_node) > 2:
            for other_node in current_node[1].split():
                edges.append((other_node, current_node[0]))

    for key, value in node_labels.items():
        strs = '\n'.join(sorted(value[1:], key=custom_sort))
        nums = value[0]
        node_labels[key] = nums + '@' + str(strs)

    return edges, node_labels


def read_file(file_path):
    try:
        with open(file_path, 'r') as file:
            content = file.read()
            print("File content:")
            print(content)
    except FileNotFoundError:
        print("File not found. Please provide a valid file path.")


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        print("Usage: python main.py <file_path> <new_file_name>")
        sys.exit(1)
    file_path = sys.argv[1]
    new_file_name = sys.argv[2]
    read_file(file_path)

    edges_, node_labels_ = get_data_from_file(file_path)
    create_directed_graph(edges_, node_labels_, new_file_name)

