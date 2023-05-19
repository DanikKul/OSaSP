import time

from parser import parse_json, fix_json, parse_request
from handlers import *

threads = []
root: str
abs_root: str


def dispatch(client_socket, request: str, path: list):
    request = parse_request(request)
    print(f'{t.current_thread().name}: [DISPATCH]: Handling request of "{request[0]}"')
    if request[0] == 'info':
        handle_info(client_socket)
    elif request[0] == 'cd':
        if len(request) == 1:
            request.append('/')
        elif len(request) > 2:
            client_socket.send("server: can't execute \"cd\" command: too many arguments".encode('utf-8'))
            return
        path[0] = handle_cd(client_socket, request[1], abs_root, path[0])
    elif request[0] == 'ls':
        handle_ls(client_socket, path[0])
    elif request[0] == 'echo':
        handle_echo(client_socket, request[1:])
    elif request[0] == 'quit':
        handle_exit(client_socket)
    else:
        handle_not_found(client_socket, request[0])


def serve(client_socket):
    path = [abs_root]
    print(f'{t.current_thread().name}: [SERVE]: Start serving')
    client_socket.send("".join(path[0].split('/')[-1:]).encode('utf-8'))
    while True:
        try:
            message = client_socket.recv(1024).decode('utf-8')
            if message == '':
                break
            else:
                print(f'{t.current_thread().name}: [SERVE]: Received from client:', message)
                dispatch(client_socket, message, path)
        except:
            break
    print(f'{t.current_thread().name}: [SERVE]: Client disconnected')
    client_socket.close()


def cleaner():
    print(f'{t.current_thread().name}: [CLEANER]: Started cleaner')
    while True:
        to_clean = []
        for i in range(len(threads)):
            print(f'{t.current_thread().name}: [CLEANER]: {threads[i].name} is {"alive" if threads[i].is_alive() else "dead"}')
            if not threads[i].is_alive():
                to_clean.append(i)
        for i in to_clean:
            try:
                print(f'{t.current_thread().name}: [CLEANER]: Cleaning {threads[i].name}')
                threads.pop(i)
            except:
                pass
        time.sleep(5)


def run_server(config):

    service_thread = t.Thread(target=cleaner, daemon=True)
    service_thread.start()

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server_address = (config['address'], config['port'])
    server_socket.bind(server_address)

    server_socket.listen(config['max_clients'])
    print('[SERVER]: Server is running and listening for connections...')

    while True:
        client_socket, client_address = server_socket.accept()
        print('[SERVER]: Accepted connection from:', client_address)
        if len(threads) >= config['max_clients']:
            client_socket.send("[SERVER]: Server is full".encode('utf-8'))
        else:
            threads.append(t.Thread(target=serve, args=(client_socket,), daemon=True))
            threads[-1].start()
        if len(threads) - 1 == config['max_clients']:
            print('[SERVER]: Server is full')


if __name__ == "__main__" :
    conf = parse_json('./config/config.json')
    fix_json(conf)
    abs_root = conf['abs_root']
    root = conf['root']
    run_server(conf)
