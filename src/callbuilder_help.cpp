/***************************************************************************
 *   Copyright (C) 2007 by PAX   *
 *   pax@m-200.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
char help_str[] =
"\n\
Настройка CALLBUILDER осуществляется с помощью командной строки в следующем формате:\n\
\n\
 callbuilder [ -tfsfile _file_ | -tfsdir _dir_ | -spiderip _ip_addr_ ] -spiderport _n_port_ -serverport _n_port_ -outdir _dir_ -rotation N -cdrformat N -nonumstr _str_ -convert -str2boff -nojornal -remrm3 -logfile _name_ -in x-y -d -radaccip _ip_addr_ -radaccsecret _secret_ -radict _file_\n\
\n\
 Например:\n\
\n\
 /usr/local/sbin/callbuilder -spiderip localhost -spiderport 10002 -outdir /var/tarif/cdr -rotation 1 -convert logfile /dev/null -radaccip 192.168.5.123 -radaccsecret secret -radict /etc/radius/dictionary -d\n\
\n\
 Возможные параметры командной строки:\n\
\n\
 1) -tfsfile _file_ - tfs фаил для обработки;\n\
 2) -tfsdir _dir_ - tfs директория для обработки. Можно обработать все файлы в конкретной директории. Функция доступна только для OS LINUX (не FREE_BSD и WINDOWS);\n\
 3) -spiderip _ip_addr_ - IP адрес для подключения к SPIDER. Обязательный параметр. Программа работает в двух режимах (файловый и сетевой). В случае выбора файлового режима следует не указывать параметр -spiderip;\n\
 4) -spiderport _n_port_ - порт, предоставляемый spider для подключения клиентов. Обязательный параметр;\n\
 5) -serverport _n_port_ - Порт сервера Callbuilder для получения cdr строк по tcp. Cтроки журнала jrn доступны по\n\
 порту serverport+1, а строки ошибок err по порту serverport+2. По умолчанию равен порт spiderport+1;\n\
\n\
 6) -ss - обработать tfs фаилы с SS коммутатора (рекомендуется выставлять в любом случае если работаем с SS коммутатором)\n\
 7) -outdir _dir_ - путь для сохраняемых err, jrn и log файлов;\n\
 8) -rotation N период ротации файлов;\n\
    1 по дням;\n\
    2 по декадам;\n\
    3 по месяцам;\n\
    4 realtime, только терминальный вывод на указанный порт (файлы не пишутся);\n\
 9) -cdrformat N - Номер формата CDR строк;\n\
    2 Тариф 2004 (по умолчанию);\n\
    10 Расширенный формат;\n\
    20 BGBilling\n\
 10) -a164 _a164_ - префикс для A номера при выбранном формате BGBilling\n\
 11) -b164 _b164_ - префикс для B номера при выбранном формате BGBilling\n\
 12) -nonumstr _str_ - Задаёт строку разделитель в случае отсутствующего номера абонента или АОН (по умолчанию \"-\");\n\
 13) -convert - создавать err и jrn файлы в формате KOI8;\n\
 14) -str2boff - Не включать в начало строки log файла бинарную двойку;\n\
 15) -nojournal - Отключить поддержку ведения журнала;\n\
 16) -remrm3 - Удалять входные rm3 файлы при удачной обработке;\n\
 17) -maxduration N - максимальная продолжительность разговора, после которой звонки попадают в журнал (по умолчанию 20000 сек.);\n\
 18) /0 - Не фиксировать неотвеченные вызовы;\n\
 19) /1 - Вырезать первую цифру из АОН;\n\
 20) /q - Уменшать длительность каждого разговора на 1 сек;\n\
 21) -prefix _n_ - префикс для внутренних номеров;\n\
 22) -logfile _name_ - Задаёт полное имя для log-файла сallbuilder. По умолчанию фаил callbuilder.log пишется в outdir;\n\
 23) -in x-y - Задаёт диапазон внутренних номеров (x <= y). Эти номера также используюстя для RADIUS авторизации;\n\
 24) -filename _filename_ - Задаёт имя для cdr файла сallbuilder (вместо cdr_log);\n\
 25) -d запускаться в режиме демона;\n\
 26) -radaccip _ip_addr_ - IP адрес RADIUS accounting сервера;\n\
 27) -radaccport N - udp порт RADIUS accounting сервера (по умолчанию 1813);\n\
 28) -radaccsecret _secret_ - секрет RADIUS accounting сервера;\n\
 29) -radaccbill N - тип биллинговой системы акаунтинга;\n\
    0 - LanBilling (по умолчанию);\n\
    1 - UTM5;\n\
 30) -radauthip _ip_addr_ - IP адрес RADIUS authorization сервера;\n\
 31) -radauthport N - udp порт RADIUS authorization сервера (по умолчанию 1812);\n\
 32) -radauthsecret _secret_ - секрет RADIUS authorization сервера;\n\
 33) -radauthbill N - тип биллинговой системы авторизации (может принимать такие же значения как и парамерт \"-radaccbill\");\n\
 34) -scommip _ip_addr_ - IP адрес scomm (нужен для подключения к scomm c целью отбоя абонента в случае неуспешной авторизации);\n\
 35) -scommport N - tcp порт прослушиваемый scomm;\n\
 36) -scommpassw _passw_ - пароль для перевода АТС m-200 в бинарный режим при подключении через scomm (по умолчанию \"100100\");\n\
 37) -radconf _file_ - фаил конфигурации RADIUS клиента callbuilder (по умолчанию не используется. Если задан, то отменяет все остальные параметры командной строки для RADIUS клиента callbuilder);\n\
 38) -raddict _file_ - фаил словаря RADIUS клиента callbuilder (по умолчанию \"dictionary\", и должен находиться в текущей директории программы);\n\
 39) -radseq _file_ - фаил для хранения линейно возрастающего номера последовательности RADIUS запрсов (по умолчанию /var/run/radius.seq);\n\
 40) -radretry N - число повторных пересылок RADIUS запросов\n\
 41) -ratimeout N - время в секундах между повторными пересылками RADIUS запросов\n\
 42) -f _file_ - фаил конфигурации callbuilder (дополняет параметры командной строки); \n\
 43) -radauthprepay - проводить RADIUS авторизацию по схеме prepay. Схема prepay обеспечивает удержание процесса обработки (\"парковку\") вызова до окончания авторизации абонента на RADIUS сервере. Данная схема авторизации поддерживается абонентами m-200 (SIG_EXT) в модулях аналоговых линий и специальными портами авторизации (SIG_AUTH) в коммутаторах серии SS; \n\
 44) -notarif - не тарифицировать вызовы. Может использоваться совместно с параметром -radauthprepay, когда нет необходимости тарифицировать вызовы (только авторизация по схеме prepay);\n\
 45) -logsize - максимальный размер log файла в мегабайтах (по умолчанию 20 МБ, не более 100 МБ);\n\
 46) -radtrace - трассировка по радиусу в log фаил;\n\
 47) -h - справка.\n\
";
