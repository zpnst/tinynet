## Сетевая модель интерфейса ОС Linux

TINYNET – модель компьютерной сети, где каждый узел представляет собой отдельный процесс в операционной системе, межпроцессное взаимодействие осуществляется через сокеты домена UNIX, в модели реализована статическая маршрутизация на основе алгоритма Флойда — Уоршелла по поиску кратчайших путей в графе.
## Что, почему  и как?

Топология сети задаётся в конфигурационном файле формата yaml. В рамках роутеров можно задавать разные топологии: mesh(полносвязная сеть), bus(шина) и ring(кольцо).
Картинки под каждый из видов топологии сети будут показаны в разделе “Визулизация сети”. Вот пример конфигурационного файла, с которым будем работать дальше:
![[vault/12.png|500]]
Для взаимодействия с приложением модели был реализован простейший интерфейс командной строки
![[vault/1.png|600]]
При запуске программы всё начинается c Network Manager – главного процесса модели, который порождает все остальные процессы сетевых устройств и хостов. Для того, чтобы наглядно ознакомится со всеми вышесказанным, стоит обратить на PID(Process ID) в командной строке, которым Network Manager любезно делится.
![[vault/2.png|600]]
Вот он. Это число – уникальный идентификатор процесса, с помощью стандартной утилиты pstree можно посмотреть дерево запущенных на данный момент процессов, начиная с желаемого.
![[vault/3.png|600]]
Как мы видим, при запуске программы создается процесс только для Network Manager

Инициализировать сеть можно с помощью команды net up
![[vault/4.png|600]]
Network manager логгирует информацию о запущенных процессах устройств и  о путях к их сокетам в файловой системе(об этом чуть позже).

Давайте  инициализируем сеть с помощью команды net up и посмотрим как изменится вывод команды pstree
![[vault/5.png|600]]
Network Manager запустил 14 процессов устройств, для которых он стал родителем

После запуска всех процессов в файловой системе по пути /tmp/tinynet создаются папки под каждое из устройств. В каждой из папок хранится сокет и файл message.log(у хостов), который появляется после получения хостом первого входящего сообщения и хранит само сообщение и путь, который это сообщение прошло(посмотрим на это чуть позже). 

![[vault/6.png|600]]

Сокет домена UNIX - это, говоря простым языком, точка связи с конкретным процессом. Каждый процесс выступает как в роли “клиента”, так и в роли “сервера”. Сокет - это просто некоторая структура в памяти ядра операционной системы, но для прикладного программиста(пользователя системы), данная сущность представляется в виде файла, что является фундаментальной абстракцией всех UNIX-подобных систем. Создавая программу, мы абстрагируемся от структуры самого сокета и его внутреннего устройства(структуры в прямом смысле, в коде ядра – это просто структуры на языке С, пусть даже запутанная и громоздкая) и работаем с ним как с обычным файлом посредством оболочек над системными вызовами read() и write() из стандартной библиотеки языка C, что нереально удобно!

Все устройства слушают ассоциированный с ними конкретный сокет, каждый из которых имеет уникальный путь в файловой системе. То есть устройство(процесс), грубо говоря, ждёт пока какое-либо другое устройство(процесс) обратится к сокету(т.е файлу), по которому ждёт соединений(слушает) наше устройство. Получив какие-либо данные(массив байт) по этому сокету – устройство преобразует это в понимаемый им вид и смотрит кто должен быть получателем этих данных, далее, по своей таблице маршрутизации определяет на какое устройство(т.е на сокет какого из устройств) переслать эти данные дальше, чтобы они достигли получателя. Данные таблицы формируются процессом Network Manager с помощью алгоритма Флойда — Уоршелла, чтобы сделать путь наименьшим из возможных, исходя из весов, которые формируются случайным образом(нагляднее будет дальше) и передаются каждому из устройств при их запуске.

Давайте же отправим сообщения с помощью CLI сетевого менеджера, посмотрим на вывод и на файл логов хоста-получателя, о котором было сказано выше.
![[vault/7.png|600]]

Тут Network Manager приказал хосту host_111 переслать сообщение ‘Hello from 111, how are you?? =)’ хосту host_313. Каждый из процессов(устройств) по пути десериализовал пакет и вывел информацию о полученном сообщении и о том, куда он собирается переслать его дальше. Да, все процессы пишут в один терминал, так как процессы устройств наследуют контекст(какую-то системную информацию) от процесса Network Manager, который был запущен через терминал, а у запущенных через терминал процессов STDOUT(стандартный поток вывода) по умолчанию идёт в этот же терминал(это если вкратце).

![[vault/8.png|600]]
Как мы видим, в файле логов у хоста host_313 появилось это сообщение и путь, которое оно прошло, прежде чем добраться до процесса host_313

Отправим ещё несколько сообщений на этот хост:
![[vault/9.png|600]]
Команда net down завершает все процессы устройств, отправляя им сигнал SIGTERM.
![[vault/10.png|600]]

Командой quit завершается сам процесс Network Managera’a.
![[vault/11.png|600]]
## Визулизация сети

С помощью команды dump можно визуализировать сеть всеми возможными способами, а именно: картинкой, списком всех возможных путей,  разноцвтным списком связей и таблицами коммутации и маршрутизации.
### Список связей
![[vault/18.png]]
### Картинка топологии
![[vault/19.png]]
### Пути и таблицы
![[vault/20.png]]

Рассмотрим картинки под каждый из видов топологии, для простоты возьмём конфигурацию с четырьмя роутерами, так как топологию мы формируем только на уровне роутеров и чтобы не захламлять картинку другими устройствами.

![[vault/14.png]]

Напомню, что веса в графе формируются случайным образом как число от 1 до 100
### Mesh
![[vault/15.png]]

### Ring
![[vault/16.png]]

### Bus
![[vault/17.png]]