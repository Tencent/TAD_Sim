# Copyright (c) 2024, Tencent Inc.
# All rights reserved.
import xml.etree.ElementTree as ET
import os
import sys
# import shutil

# 备注:
# 参数:
# 返回值:


def upgrade_01(root):
    root.attrib['version'] = '1.0'
    return root


# input function
if __name__ == '__main__':
    args = sys.argv
    if len(args) < 3:
        print('input sys dir and user dir')
        sys.exit(1)

    sys_dir = args[1]
    user_dir = args[2]

    if not os.path.exists(os.path.join(user_dir, 'data/scenario/sensors/config.json')):
        print('config.json donot exists')
        sys.exit(-1)
        # shutil.copy(os.path.join(sys_dir, 'scenario/sensors/config.json'),
        # os.path.join(user_dir, 'scenario/sensors/config.json'))

    if not os.path.exists(os.path.join(user_dir, 'data/scenario/sensors/sensor_global.xml')):
        print('sensor_global.json donot exists')
        sys.exit(-1)
        # shutil.copy(os.path.join(sys_dir, 'scenario/sensors/sensor_global.xml'),
        # os.path.join(user_dir, 'scenario/sensors/sensor_global.xml'))
    else:
        tree = ET.parse(os.path.join(user_dir, 'data/scenario/sensors/sensor_global.xml'))
        root = tree.getroot()
        old_version = root.attrib['version']

        while True:
            version = root.attrib['version']
            if version == '0.1':
                root = upgrade_01(root)
            elif version == '1.0':
                break
            else:
                print('wrong version')
                break

        new_version =  root.attrib['version']
        if old_version != new_version:
            tree.write(os.path.join(user_dir, 'data/scenario/sensors/sensor_global.xml'),
                       encoding='utf-8', xml_declaration=True)
