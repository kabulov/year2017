
from contextlib import redirect_stdout
from virtual_machine import VirtualMachine

test_names = None
with open("test_names", "r") as names:
    test_names = [line.strip() for line in names]

exec_stdout = "exec_result"
vm_stdout = "vm_result"

exec_result = None
vm_result = None

compare_results = "tests_compare_results"

with open(compare_results, "w") as file:
    with redirect_stdout(file):
        print("test amount = " + str(len(test_names)), end='\n\n')

for one_test_name in test_names:
    source_code = None
    with open(one_test_name, "r") as test_file:
        source_code = test_file.read()
    code_object = compile(source_code, one_test_name, 'exec')

    with open(exec_stdout, "w") as file:
        with redirect_stdout(file):
            exec(code_object)

    virtual_machine = VirtualMachine()
    with open(vm_stdout, "w") as file:
        with redirect_stdout(file):
            virtual_machine.run_code(code_object)

    with open(exec_stdout, "r") as file:
        exec_result = file.read()

    with open(vm_stdout, "r") as file:
        vm_result = file.read()

    with open(compare_results, "a") as file:
        with redirect_stdout(file):
            print(one_test_name, end=' = ')
            if (exec_result == vm_result):
                print('OK')
            else:
                print('FAIL')

#TODO:
#1. handle exceptions
#2. redirect stderr (goto error_print.py)
#3. compare stderrs
