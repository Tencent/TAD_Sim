import argparse
import csv
import json

# 用法，将两个输入的文件和本python放到同一目录，或是下方文件名写完整路径名
# 执行python3 transfer.py --csv <源our-csv> --input <your-input-json> --output <your-output-json>

# 解析器创建
parser = argparse.ArgumentParser(description='Process some files.')
# 添加参数
parser.add_argument('--csv', type=str, help='The csv file', default='source.csv')
parser.add_argument('--input', type=str, help='The input json file', default='../all_grading_kpi.json')
parser.add_argument('--output', type=str, help='The output json file', default='target.json')
# 解析参数
args = parser.parse_args()
# 将参数赋值给变量
excel_file_name = args.csv
source_json_name = args.input
target_json_name = args.output

wanted_kpis = []

with open(excel_file_name, 'r') as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        if len(row) > 1:
            wanted_kpis.append(row[1])

with open(source_json_name, 'r', encoding='utf-8') as jsonfile:
    source_data = json.load(jsonfile)
    source_data['kpi'] = [item for item in source_data['kpi'] if item['name'] in wanted_kpis]

with open(target_json_name, 'w', encoding='utf-8') as jsonfile:
    json.dump(source_data, jsonfile, ensure_ascii=False, indent=2)
