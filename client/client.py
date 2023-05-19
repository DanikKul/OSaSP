import socket
from parser import parse_json, fix_json


def run_file(client_socket: socket.socket, request: str, path: str):
    try:
        with open(request) as file:
            commands = file.readlines()
            print(commands)
            for command in commands:
                if command[-1] == '\n':
                    command = command[:-1]
                print(f"{path}> {command}")
                if command == "":
                    continue

                if command.split(' ')[0] == 'run':
                    print(f"[CLIENT]: Recursion warning: Disallowed to run file from file")
                    continue

                client_socket.send(command.encode('utf-8'))

                if command == 'quit':
                    client_socket.close()
                    exit(0)

                response = client_socket.recv(1024).decode('utf-8')
                if response == '':
                    client_socket.close()
                    exit()
                if command[:2] == "cd":
                    path = response
                else:
                    print(response)
    except:
        print('[CLIENT]: Invalid path to file')

    return path


def run_client(config: dict):
    # Create a TCP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect to the server
    server_address = (config['address'], config['port'])
    client_socket.connect(server_address)
    path = client_socket.recv(1024).decode('utf-8')

    if path == "[SERVER]: Server is full":
        print(path)
        client_socket.close()
        return

    while True:
        # Send message to the server
        message = input(f"{path}> ")
        if message == "":
            continue

        if message.split(' ')[0] == 'run':
            if len(message.split(' ')) == 1:
                print("[CLIENT]: No path to file provided to run")
            elif len(message.split(' ')) > 2:
                print('[CLIENT]: Too many arguments')
            else:
                path = run_file(client_socket, message.split(' ')[1], path)
            continue

        client_socket.send(message.encode('utf-8'))

        if message == 'quit':
            break

        # Receive response from the server
        response = client_socket.recv(1024).decode('utf-8')
        if response == '':
            break
        if message[:2] == "cd":
            path = response
        else:
            print(response)

    # Close the client socket
    client_socket.close()


if __name__ == "__main__":
    conf = parse_json('./config/config.json')
    fix_json(conf)
    run_client(conf)
