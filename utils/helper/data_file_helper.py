from __future__ import absolute_import, division, print_function, unicode_literals

import sys
import os.path
from model.storage import *

if 2 == sys.version_info[0]:
    text = unicode
else:
    text = str


class DataFileHelper:

    KEY_INDEX_VOLUME = 'volume index:'
    KEY_INDEX_LAYER = 'layer index:'
    KEY_INDEX_PART = 'part index:'
    KEY_INDEX_OUTLINE = 'outline index:'
    KEY_SIZE_MODEL = 'model size:'

    FILE_NAME_PARTS = text('parts_by_layers.txt')
    FILE_NAME_POINTS_PAIRS = text('parts_points_pairs.txt')

    def __init__(self):
        pass

    @staticmethod
    def load_data_from_dir(file_dir):
        """
        load parts data and point pairs data from two files in directory
        :param file_dir: text string of file directory
        :return: a Storage object
        """
        assert isinstance(file_dir, text)
        if not os.path.isdir(file_dir):
            return None, 'Invalid directory %s!' % file_dir
        storage, msg = DataFileHelper.load_part_file(
            os.path.join(file_dir, DataFileHelper.FILE_NAME_PARTS))
        if storage is None:
            return None, msg
        point_pairs_file_path = os.path.join(file_dir, DataFileHelper.FILE_NAME_POINTS_PAIRS)
        storage, msg = DataFileHelper.load_point_pairs_file(point_pairs_file_path, storage)
        return storage, msg

    @staticmethod
    def load_point_pairs_file(file_path, storage):
        """
        load point pairs file into memory
        :param file_path: file path to point pairs data file
        :param storage: a loaded storage object
        :return: a storage object with entry point and exit point in each part
        """
        assert isinstance(file_path, text)
        if not os.path.isfile(file_path):
            return None, "file %s doesn't exist!" % file_path
        assert isinstance(storage, Storage)
        # initialize index
        volume_index = None
        layer_index = None
        part_index = None

        with open(file_path, 'r') as part_file:
            for line in part_file:
                if line.startswith(DataFileHelper.KEY_INDEX_VOLUME):
                    volume_index = int(line[len(DataFileHelper.KEY_INDEX_VOLUME):])
                elif line.startswith(DataFileHelper.KEY_INDEX_LAYER):
                    layer_index = int(line[len(DataFileHelper.KEY_INDEX_LAYER):])
                elif line.startswith(DataFileHelper.KEY_INDEX_PART):
                    part_index = int(line[len(DataFileHelper.KEY_INDEX_PART):])
                else:
                    # read point list into polygon
                    str_list = line.split()
                    num_list = [int(e) for e in str_list]
                    assert len(num_list) == 4
                    volume = storage.get_volume(volume_index)
                    layer = volume.get_layer(layer_index)
                    part = layer.get_part(part_index)
                    part.entry_point.x = num_list[0]
                    part.entry_point.y = num_list[1]
                    part.exit_point.x = num_list[2]
                    part.exit_point.y = num_list[3]
        return storage, 'Succeeded in loading point pairs from data file!'

    @staticmethod
    def load_part_file(file_path):
        """
        load part file into memory
        :param file_path: file path to parts data file
        :return: a Storage object
        """
        assert isinstance(file_path, text)
        if not os.path.isfile(file_path):
            return None, "file %s doesn't exist!" % file_path
        storage = Storage()
        # initialize index
        volume_index = None
        layer_index = None
        part_index = None
        outline_index = None

        current_volume = None
        current_layer = None
        currnet_part = None
        model_size_x = None
        model_size_y = None

        with open(file_path, 'r') as part_file:
            for line in part_file:
                if line.startswith(DataFileHelper.KEY_SIZE_MODEL):
                    # read model size
                    str_list = line[len(DataFileHelper.KEY_SIZE_MODEL):].split()
                    num_list = [int(e) for e in str_list]
                    model_size_x = num_list[0]
                    model_size_y = num_list[1]
                elif line.startswith(DataFileHelper.KEY_INDEX_VOLUME):
                    volume_index = int(line[len(DataFileHelper.KEY_INDEX_VOLUME):])
                    current_volume = Volume()
                    storage.add_volume(current_volume)
                elif line.startswith(DataFileHelper.KEY_INDEX_LAYER):
                    layer_index = int(line[len(DataFileHelper.KEY_INDEX_LAYER):])
                    current_layer = Layer()
                    current_volume.add_layer(current_layer)
                elif line.startswith(DataFileHelper.KEY_INDEX_PART):
                    part_index = int(line[len(DataFileHelper.KEY_INDEX_PART):])
                    current_part = Part()
                    current_layer.add_part(current_part)
                elif line.startswith(DataFileHelper.KEY_INDEX_OUTLINE):
                    outline_index = int(line[len(DataFileHelper.KEY_INDEX_OUTLINE):])
                else:
                    # read point list into polygon
                    str_list = line.split()
                    num_list = [int(e) for e in str_list]
                    polygon = Polygon()
                    if len(num_list) % 2 != 0:
                        raise RuntimeError('Invalid number of the point list -- %d!')
                    for point_index in range(int(len(num_list)/2)):
                        polygon.add_point(num_list[2 * point_index], num_list[2 * point_index + 1])
                    current_part.add_outline(polygon)

            storage.model_size_y = model_size_y
            storage.model_size_x = model_size_x

        return storage, 'succeeded in loading part data!'


