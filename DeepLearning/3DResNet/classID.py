
class_id_to_name_dic ={
    "0": "4379243",
    "1": "3001627",
    "2": "2691156",
    "3": "2958343",
    "4": "4256520",
    "5": "4090263",
    "6": "3636649",
    "7": "4530566",
    "8": "2828884"
}


class_name_to_id_dic = { v : k for k, v in class_id_to_name_dic.items() }
class_names = set(class_id_to_name_dic.values())

def class_name_to_id(class_names_list):
    ID=[]
    for name in class_names_list:
        ID.append(class_name_to_id_dic[name])
    return ID

