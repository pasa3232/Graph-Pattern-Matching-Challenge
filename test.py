import subprocess
import time


def score():
    f = open("result.txt", "r")
    s = f.read()
    f.close
    return s.count("a")

def test(MODE, data):
    print(f"<{data}>")
    cases = ["n1", "n3", "n5", "n8", "s1", "s3", "s5", "s8"]
    for case in cases:
        print(f"{data}_{case}: ", end="")
        p = subprocess.Popen([f"./build/main/program", f"data/{data}.igraph", f"query/{data}_{case}.igraph", f"candidate_set/{data}_{case}.cs", "result.txt", f"-{MODE}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            st = time.time()
            out, err = p.communicate(timeout=60)
            end = time.time()
            print(f"TIME: {round(end - st, 3):<7}(s)  ", end = "")
        except subprocess.TimeoutExpired:
            p.kill()
            print(f"TIME: {'(time out)':<10}  ", end = "")
        print(f"SCORE: {score()}")
    print()

    
print("\n=================== ignore dag2 ====================")
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


print("\n=================== ignore dag ===================")
test(4, "lcc_hprd")
test(4, "lcc_human")
test(4, "lcc_yeast")
print()
