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
- Для проверки работы резолва симлинков создайте симлинки сами.

### Пример создания симлинка

Структура проекта:
```
. /client
. /server
. /scripts
.  README.md
```
Создайте папку/файл
```
touch blablabla.txt # Для создания файла
или
mkdir folder # Для создания папки
```
Создайте симлинк на файл
```
ln -s /path/to/folder folder_sl
```
Полученная структура проекта:
```
. /client
. /server
. /scripts
. /folder
. /folder_sl -> /folder
.  blablabla.txt
.  README.md
```
Учтите, что для создания симлинка должен быть указан полный путь к файлу/папке
