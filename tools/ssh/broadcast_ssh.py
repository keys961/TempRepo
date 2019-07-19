import paramiko
import sys


if __name__ == "__main__":
    client_map = {}
    hosts = ["localhost"]
    for host in sys.argv[1:]:
        hosts.append(host)
    for host in hosts:
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(host, username="root", timeout=10)
        print("{} is connected successfully.".format(host))
        client_map[host] = client
    while True:
        command = input("# ")
        if command == "exit":
            break
        if command.startswith("cd"):
            command = "{}; pwd".format(command)
        for host, client in client_map.items():
            cmd_in, cmd_out, cmd_err = client.exec_command(command)
            out = cmd_out.readlines()
            print("==========={}==========".format(host))
            for o in out:
                print("[{}]: {}".format(host, o), end="")
            print("====================================")
    for client in client_map.values():
        client.close()
