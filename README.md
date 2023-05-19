# Instructions

Зайдите в папку со скриптами
  ```
  cd /path/to/lab/scripts
  ```
Запустите скрипт запуска сервера
  ```
  bash start_server.sh
  ```
Запустите скрипт запуска клиента
  ```
  bash start_client.sh
  ```

# Notes
- В /server/config/config.json перепишите root и abs_root на свою папку, которую хотите сделать корневой на сервере
- В /server/config/config.json и /client/config/config.json перепишите адрес и порт на те, которые вы захотите (localhost = 127.0.0.1)
- Естественно, чтобы лабороторная заработала нужно установить Python версии 3.X.X
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
