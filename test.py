import subprocess
import time


def score():
    f = open("result.txt", "r")
    s = f.read()
    f.close
    return s.count("a")

def test(MODE, data):
    print(f"<{data}>")
    for num in range(1, 9, 2):
        print(f"{data}_n{num}: ", end="")
        p = subprocess.Popen([f"./build/main/program", f"data/{data}.igraph", f"query/{data}_n{num}.igraph", f"candidate_set/{data}_n{num}.cs", "result.txt", f"-{MODE}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out, err = p.communicate(timeout=60)
        except subprocess.TimeoutExpired:
            p.kill()
            print("(time out)", end="")
        print(score())

    for num in range(1, 9, 2):
        print(f"{data}_s{num}: ", end="")
        p = subprocess.Popen([f"./build/main/program", f"data/{data}.igraph", f"query/{data}_s{num}.igraph", f"candidate_set/{data}_s{num}.cs", "result.txt", f"-{MODE}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out, err = p.communicate(timeout=60)
        except subprocess.TimeoutExpired:
            p.kill()
            print("(time out)", end="")
        print(score())
    print()

    

print("\n================= TEST ignore dag =================")
test(0, "lcc_hprd")
test(0, "lcc_human")
test(0, "lcc_yeast")
print()

print("\n======================= DAF =======================")
test(1, "lcc_hprd")
test(1, "lcc_human")
test(1, "lcc_yeast")
print()

print("\n====================== elpsm ======================")
test(2, "lcc_hprd")
test(2, "lcc_human")
test(2, "lcc_yeast")
print()

print("\n=================== optimize-da ===================")
test(3, "lcc_hprd")
test(3, "lcc_human")
test(3, "lcc_yeast")
print()
