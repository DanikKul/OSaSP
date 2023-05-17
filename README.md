# Instructions
## Способ 1

Перейдите в папку scripts
  ```
  cd /path/to/lab/scripts
  ```
Запустите скрипт сборки сервера
  ```
  bash build_server.sh
  ```
Запустите скрипт сборки клиента
  ```
  bash build_client.sh
  ```
Запустите сервер и введите порт
  ```
  bash start_server.sh
  ```
Запустите клиента, введите адрес и порт сервера
  ```
  bash start_client.sh
  ```

# Notes
- В /server/handlers.c редефайните ROOT на свою папку, которую хотите сделать корневой на сервере
