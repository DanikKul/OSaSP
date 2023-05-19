import threading as t
import socket
import os
import sys


def handle_info(client_socket: socket.socket):
    print(f'{t.current_thread().name}: [INFO]: Executing "info"')
    client_socket.send("Some info about server".encode('utf-8'))
    print(f'{t.current_thread().name}: [INFO]: Done')


def handle_not_found(client_socket: socket.socket, request: str):
    print(f'{t.current_thread().name}: [NOT FOUND]: Executing "handle_not_found"')
    client_socket.send(f"server: command not found: {request}".encode('utf-8'))
    print(f'{t.current_thread().name}: [NOT FOUND]: Done')


def handle_exit(client_socket: socket.socket):
    print(f'{t.current_thread().name}: [EXIT]: Executing "exit"')
    client_socket.close()
    print(f'{t.current_thread().name}: [EXIT]: Terminating session')
    sys.exit(0)


def handle_echo(client_socket: socket.socket, request: list):
    print(f'{t.current_thread().name}: [ECHO]: Executing "echo"')
    if len(" ".join(request).replace('"', '', -1).replace("'", '', -1)) == 0:
        print(f'{t.current_thread().name}: [ECHO]: Can\'t parse string {" ".join(request)}')
        client_socket.send(" ".encode('utf-8'))
        print(f'{t.current_thread().name}: [ECHO]: Done')
        return
    client_socket.send(" ".join(request).replace('"', '', -1).replace("'", '', -1).encode('utf-8'))
    print(f'{t.current_thread().name}: [ECHO]: Done')


def handle_cd(client_socket: socket.socket, request: str, abs_root: str, current: str):
    print(f'{t.current_thread().name}: [CD]: Executing "cd" with arg {request}, current path is {current}')
    if request == '/':
        out = os.path.realpath(abs_root)
        client_socket.send("".join(out.split('/')[-1:]).encode('utf-8'))
        print(f'{t.current_thread().name}: [CD]: Path changed to {out}')
        print(f'{t.current_thread().name}: [CD]: Done')
        return out
    path = current
    paths = request.split('/')
    out: str
    for p in paths:
        everything = os.listdir(path)
        everything.append('.')
        everything.append('..')
        if os.path.realpath(abs_root) == os.path.realpath(path) and p == '..':
            print(f'{t.current_thread().name}: [CD]: Can\'t go lower than root path')
            client_socket.send("".join(current.split('/')[-1:]).encode('utf-8'))
            print(f'{t.current_thread().name}: [CD]: Done')
            return current
        if p in everything:
            if path[-1] != '/':
                path += '/'
            path += p
    out = os.path.realpath(path)
    client_socket.send("".join(out.split('/')[-1:]).encode('utf-8'))
    print(f'{t.current_thread().name}: [CD]: Path changed to {out}')
    print(f'{t.current_thread().name}: [CD]: Done')
    return out


def handle_ls(client_socket: socket.socket, path: str):
    print(f'{t.current_thread().name}: [LS]: Executing "ls"')
    everything = os.listdir(path)
    out = ""
    if path[-1] != '/':
        path += '/'
    for i in everything:
        if os.path.isdir(path + i):
            out += i + '/\n'
        elif os.path.islink(path + i):
            tmp = os.readlink(path + i)
            if os.path.islink(tmp):
                out += i + ' -->> ' + "/".join(os.path.realpath(path + i).split('/')[-2:]) + '\n'
            else:
                out += i + ' --> ' + "/".join(os.path.realpath(path + i).split('/')[-2:]) + '\n'
        else:
            out += i + '\n'
    out = out[:-1]
    client_socket.send(out.encode('utf-8'))
    print(f'{t.current_thread().name}: [LS]: Done')
