import sys
import matplotlib
import os
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.path as mpltPath
import tkinter.ttk as TTK
import tkinter as TK
from tkinter import messagebox as MB
from tkinter import *
from tkinter.ttk import *
from tkinter import filedialog as FD
from helper.data_file_helper import DataFileHelper

# For Python 2.x compatible, use the same string type
text = str

# Only support Python3
if 2 == sys.version_info[0]:
    sys.stderr.write('UI part viewer requires Python 3.x!')
    sys.exit(1)

# matplotlib.use("TkAgg")


class MainApp:
    DEFAULT_DIR_PATH = text('/tmp')
    DEFAULT_WINDOWS_TITLE = text('Parts Viewer')

    def __init__(self):
        # initialized variables
        self._storage = None
        self._dir_path = None
        self._layer_index = None
        self.file_path_lable = None
        # create widgets
        self._create_widgets()
        # load data by default file path
        self.load_data(MainApp.DEFAULT_DIR_PATH)

    def _create_widgets(self):
        # create master
        self.master = Tk()
        self.master.title(MainApp.DEFAULT_WINDOWS_TITLE)
        # hook up window delete event
        self.master.protocol("WM_DELETE_WINDOW", self.quit)

        # create widgets
        self._create_panel_main()

        # fill in screen
        w = self.master.winfo_screenwidth()
        h = self.master.winfo_screenheight()
        self.master.geometry("%dx%d+0+0" % (w, h))

    def _create_panel_main(self):
        # define var
        self.selected_layer_index = IntVar()
        self.enable_legend = IntVar()
        self.enable_part_num = IntVar()
        self.enable_part_order = IntVar()

        self.file_panel = Frame(self.master, name="file")
        self.file_panel.grid(row=0, column=0)
        # file panel
        open_file_button = Button(self.file_panel, text='Open data directory...', command=self.ask_data_dir_dialog)
        open_file_button.grid(row=0, column=0, padx=20, pady=5, sticky=E)

        refresh_button = Button(self.file_panel, text='Refresh', command=lambda: self.load_data(self._dir_path))
        refresh_button.grid(row=0, column=1, padx=20, pady=5, sticky=E)

        legend_check_button = Checkbutton(self.file_panel, text="Show Legend", variable=self.enable_legend,
                                          command=lambda: self.refresh_graph(self.selected_layer_index.get() - 1))
        legend_check_button.grid(row=0, column=2, sticky=W)

        part_num_check_button = Checkbutton(self.file_panel, text="Show Part Number", variable=self.enable_part_num,
                                            command=lambda: self.flip_part_num_check_button(self.selected_layer_index.get() - 1))
        part_num_check_button.grid(row=0, column=3, sticky=W)

        part_order_check_button = Checkbutton(self.file_panel, text="Show Part Order", variable=self.enable_part_order,
                                              command=lambda: self.flip_part_order_check_button(self.selected_layer_index.get() - 1))
        part_order_check_button.grid(row=0, column=4, sticky=W)

        # plot
        self.figure = Figure()
        self.ax = self.figure.add_subplot(111, aspect='equal')

        self.canvas = FigureCanvasTkAgg(self.figure, master=self.master)
        self.canvas.draw()
        self.canvas.get_tk_widget().grid(row=1, column=0, sticky=N+S, padx=5, pady=5)

        # grid layout
        self.master.columnconfigure(0, weight=1)
        self.master.rowconfigure(1, weight=1)

        # scale
        self.scale = TK.Scale(self.master, from_=1, to=255, tickinterval=0, variable=self.selected_layer_index,
                              label='Layer Index', orient="vertical", command=self.update_layer_index)

        self.scale.grid(row=1, column=1, sticky=N+S, padx=20, pady=30)

    def ask_data_dir_dialog(self):
        dir_path = FD.askdirectory()
        if dir_path is not None and len(dir_path) != 0:
            self.load_data(dir_path)
        return dir_path

    def load_data(self, dir_path):
        if dir_path is not None and os.path.isdir(dir_path):
            self._dir_path = dir_path
            # load parts data
            self._load_parts_data(self._dir_path)
            if self._storage is not None:
                # refresh graph drawing
                self.refresh_graph()

    def _load_parts_data(self, dir_path):
        if os.path.isdir(dir_path):
            self._storage, msg = DataFileHelper.load_data_from_dir(dir_path)
            if self._storage is None:
                MB.showerror("Error", msg, parent=self.master)
            else:
                self.scale.config(to=self._storage.get_volume(0).get_layer_size())
                self.master.title(MainApp.DEFAULT_WINDOWS_TITLE + ' -- ' + self._dir_path)

    @staticmethod
    def _topological_sort(dag, multiple_outline_parts):
        """
        Topological sort DAG
        :param dag:
        :param multiple_outline_parts:
        :return:
        """
        # handle special case
        if multiple_outline_parts is None or len(multiple_outline_parts) == 0:
            return list()

        # find vertex has no parent
        root_vertex_set = {part for part in multiple_outline_parts}

        for parent, children in dag.items():
            for child in children:
                if child in root_vertex_set:
                    root_vertex_set.remove(child)

        ordered_parts = list()
        is_visited = {part: False for part in multiple_outline_parts}

        # In our case, any chidlrden has one and only one parent
        # any parent has zero, one or more than one children
        for root in root_vertex_set:
            for child in dag.get(root):
                MainApp._visit_child(child, dag, ordered_parts, is_visited)
            ordered_parts.append(root)
            is_visited[root] = True
        return list(reversed(ordered_parts))

    @staticmethod
    def _visit_child(child, dag, ordered_parts, is_visited):
        if is_visited[child]:
            return

        if dag.get(child, None) is None or len(dag.get(child)) == 0:
            ordered_parts.append(child)
            is_visited[child] = True
        else:
            for grand_child in dag.get(child):
                MainApp._visit_child(grand_child, dag, ordered_parts, is_visited)
            ordered_parts.append(child)

    @staticmethod
    def _sort_parts_for_drawing(parts):
        """
        For visualization purpose, sort parts with proper order.
        If there are more than one outline in the bigger part and other smaller parts resides in outline, make sure that
        the bigger one draw first.
        :param parts: parts in layer
        :return: a list of ordered part.
        """
        # store part index into dict
        part_index_dict = {parts[i]: i for i in range(len(parts))}
        # filter out parts with the number of outlines more than one
        multiple_outline_parts = [part for part in parts if part.get_outlines_size() > 1]
        sigle_outline_parts = [part for part in parts if part.get_outlines_size() <= 1]
        # build a DAG where vertex is a part and directed edge from part A to part B represent that inner outline of A
        # cover outer outline of B (In other word, any point of outer outline of B is inside inner outline of A
        dag = dict()
        for i in range(len(multiple_outline_parts)):
            part_i = multiple_outline_parts[i]
            i_list = list()
            for j in range(len(multiple_outline_parts)):
                if i == j:
                    continue
                for inner_outline in multiple_outline_parts[i].get_inner_outlines():
                    path = mpltPath.Path([e for e in zip(inner_outline.get_all_points_at_x(),
                                                         inner_outline.get_all_points_at_y())])
                    part_j = multiple_outline_parts[j]
                    outer_polygon_of_part_j = part_j.get_outline(0)
                    if path.contains_point(
                            [outer_polygon_of_part_j.get_point_at_x(0), outer_polygon_of_part_j.get_point_at_y(0)]):
                        i_list.append(part_j)
            dag[part_i] = i_list

        ordered_multiple_outline_parts = MainApp._topological_sort(dag, multiple_outline_parts)
        new_ordered_parts = ordered_multiple_outline_parts + sigle_outline_parts
        new_ordered_index_list = [part_index_dict[part] for part in new_ordered_parts]
        return new_ordered_index_list

    def flip_part_num_check_button(self, layer_index):
        if self.enable_part_num.get() + self.enable_part_order.get() == 2:
            self.enable_part_order.set(0)
        self.refresh_graph(layer_index)

    def flip_part_order_check_button(self, layer_index):
        if self.enable_part_num.get() + self.enable_part_order.get() == 2:
            self.enable_part_num.set(0)
        self.refresh_graph(layer_index)

    def refresh_graph(self, layer_index=0):
        self.ax.clear()
        self.ax.set_xlim(0, self._storage.model_size_x / 1000)
        self.ax.set_ylim(0, self._storage.model_size_y / 1000)
        is_first_to_draw_point = True
        for volume_index in range(self._storage.get_volumes_size()):
            layer = self._storage.get_volume(volume_index).get_layer(layer_index)
            # sort part by visualization purpose
            for part_nr in MainApp._sort_parts_for_drawing(layer.get_all_parts()):
                part = layer.get_part(part_nr)
                # draw entry point and exit point
                entry_x = part.entry_point.x / 1000
                entry_y = part.entry_point.y / 1000
                exit_x = part.exit_point.x / 1000
                exit_y = part.exit_point.y / 1000
                if is_first_to_draw_point:
                    self.ax.plot(entry_x, entry_y, 'r+', label='Entry Point')
                    self.ax.plot(exit_x, exit_y, 'bx', label='Exit Point')
                    is_first_to_draw_point = False
                else:
                    self.ax.plot(entry_x, entry_y, 'r+')
                    self.ax.plot(exit_x, exit_y, 'bx')

                for outline_nr in range(part.get_outlines_size()):
                    x = part.get_outline(outline_nr).get_all_points_at_x()
                    y = part.get_outline(outline_nr).get_all_points_at_y()
                    x = [e/1000 for e in x]
                    y = [e/1000 for e in y]
                    # end point connects to begin point
                    x.append(x[0])
                    y.append(y[0])
                    if outline_nr == 0:
                        # the first outlines filled in blue
                        fill_color = 'y'
                        if self.enable_part_num.get() == 1:
                            self.ax.text(x[0], y[0], text(part_nr))
                        if self.enable_part_order.get() == 1:
                            self.ax.text(x[0], y[0], text(part.order))
                    else:
                        fill_color = 'w'
                    self.ax.fill(x, y, fill_color)
        if self.enable_legend.get() == 1:
            self.ax.legend(loc='upper right')
        self.canvas.draw()

    def update_layer_index(self, value):
        if self._storage is not None:
            self.refresh_graph(self.selected_layer_index.get() - 1)

    def quit(self):
        self.master.destroy()

    def main_loop(self):
        self.master.mainloop()


if __name__ == '__main__':
    app = MainApp()
    app.main_loop()
