
'''
    generate info about some python code
    1. bytecode instructions list
    2. dis.dis
    3. readable dis.dis
    4. data: locals, globals, all attributes of executed code
'''

test_names = None
with open("test_names", "r") as names:
    test_names = [line.strip() for line in names]

import contextlib
import dis

for test in test_names:
    somecode = ""
    with open(test, "r") as file:
        somecode = file.read().strip()
    print(test)
    code = compile(somecode, '', 'exec')
    with open(test+"_data", "w") as file:
        with contextlib.redirect_stdout(file):
            print(list(code.co_code))
            print()
            dis.dis(code)
            print()
            b = dis.get_instructions(code)
            for elem in b:
                print(elem)
            print()
            lst = []
            for elem in dir(code):
                if not elem.startswith("__"):
                    print(elem, " : ", getattr(code, elem))
                else:
                    lst.append(elem)
            print()
            for elem in lst:
                print(elem, " : ", getattr(code, elem))
