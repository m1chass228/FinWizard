import sys 
import string
import ast

modes_dict = ast.literal_eval(sys.argv[1])     # получаем словарь с {num:info}
paths_dict = ast.literal_eval(sys.argv[2])     # получаем словарь с {num:path}
dir_on_desktop = sys.argv[3]                   # получаем папку с готовыми отчетами
json_path = sys.argv[4]                        # получаем путь к json

print(modes_dict)
print(paths_dict)
print(dir_on_desktop)
print(json_path)

