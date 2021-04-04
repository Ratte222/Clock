using System;
using System.Net;
using System.Text;
using System.IO;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;
using System.Text.RegularExpressions;

namespace GetWeather
{
    class Server
    {
        TcpListener Listener; // Объект, принимающий TCP-клиентов
        
        public enum E_DAYWEATHER
        {
            Today = 0,
            Tomorrow,
            After_tomorrow
        };

        // Запуск сервера
        public Server(int Port)
        {
            // Создаем "слушателя" для указанного порта

            Listener = new TcpListener(IPAddress.Any, Port);
            Listener.Start(); // Запускаем его

            // В бесконечном цикле
            while (true)
            {
                // Принимаем новых клиентов
                //new Client(Listener.AcceptTcpClient());
                TcpClient Client = Listener.AcceptTcpClient();
                // Создаем поток
                Thread Thread = new Thread(new ParameterizedThreadStart(ClientThread));
                // И запускаем этот поток, передавая ему принятого клиента
                Thread.Start(Client);
            }
        }

        // Остановка сервера
        ~Server()
        {
            // Если "слушатель" был создан
            if (Listener != null)
            {
                // Остановим его
                Listener.Stop();
            }
        }

        static void ClientThread(Object StateInfo)
        {
            new Client((TcpClient)StateInfo);
        }

        // Класс-обработчик клиента
        class Client
        {
            TcpClient _Client;
            // Конструктор класса. Ему нужно передавать принятого клиента от TcpListener
            public Client(TcpClient Client_)
            {
                _Client = Client_;
                //// Код простой HTML-странички
                //string Html = "<html><body><h1>It works!</h1></body></html>";
                //// Необходимые заголовки: ответ сервера, тип и длина содержимого. После двух пустых строк - само содержимое
                //string Str = "HTTP/1.1 200 OK\nContent-type: text/html\nContent-Length:" + Html.Length.ToString() + "\n\n" + Html;
                //// Приведем строку к виду массива байт
                //byte[] Buffer = Encoding.ASCII.GetBytes(Str);
                //// Отправим его клиенту
                //Client.GetStream().Write(Buffer, 0, Buffer.Length);
                //// Закроем соединение
                //Client.Close();
                HandleRequest();
            }
            private void HandleRequest()
            {
                string Request = "";
                // Буфер для хранения принятых от клиента данных
                byte[] Buffer = new byte[1024];
                // Переменная для хранения количества байт, принятых от клиента
                int Count;
                // Читаем из потока клиента до тех пор, пока от него поступают данные
                while ((Count = _Client.GetStream().Read(Buffer, 0, Buffer.Length)) > 0)
                {
                    // Преобразуем эти данные в строку и добавим ее к переменной Request
                    Request += Encoding.ASCII.GetString(Buffer, 0, Count);
                    // Запрос должен обрываться последовательностью \r\n\r\n
                    // Либо обрываем прием данных сами, если длина строки Request превышает 4 килобайта
                    // Нам не нужно получать данные из POST-запроса (и т. п.), а обычный запрос
                    // по идее не должен быть больше 4 килобайт
                    if (Request.IndexOf("\r\n\r\n") >= 0 || Request.Length > 4096)
                    {
                        break;
                    }
                }
                // Парсим строку запроса с использованием регулярных выражений
                // При этом отсекаем все переменные GET-запроса
                Match ReqMatch = Regex.Match(Request, @"^\w+\s+([^\s\?]+)[^\s]*\s+HTTP/.*|");

                // Если запрос не удался
                if (ReqMatch == Match.Empty)
                {
                    // Передаем клиенту ошибку 400 - неверный запрос
                    SendError(400);
                    return;
                }

                // Получаем строку запроса
                string RequestUri = ReqMatch.Groups[1].Value;

                // Приводим ее к изначальному виду, преобразуя экранированные символы
                // Например, "%20" -> " "
                RequestUri = Uri.UnescapeDataString(RequestUri);

                // Если в строке содержится двоеточие, передадим ошибку 400
                // Это нужно для защиты от URL типа http://example.com/../../file.txt
                if (RequestUri.IndexOf("..") >= 0)
                {
                    SendError(400);
                    return;
                }
                string response;
                Console.WriteLine(RequestUri);
                if(RequestUri.IndexOf("/getWeather_1")> -1)
                {
                    response = GetWeatherSinoptic(E_DAYWEATHER.Today);
                }
                else if (RequestUri.IndexOf("/getWeather_2") > -1)
                {
                    response = GetWeatherSinoptic(E_DAYWEATHER.Tomorrow);
                }
                else if (RequestUri.IndexOf("/getWeather_3") > -1)
                {
                    response = GetWeatherSinoptic(E_DAYWEATHER.After_tomorrow);
                }
                else if (RequestUri.IndexOf("/getDateTime") > -1)
                {
                    response = GetDateTimePC();
                }
                else
                {
                    SendError(500);
                    return;
                }
                string Html = $"{response}";
                // Необходимые заголовки: ответ сервера, тип и длина содержимого. После двух пустых строк - само содержимое
                string Str = "HTTP/1.1 200 OK\nContent-type: text/html\nContent-Length:" + Html.Length.ToString() + "\n\n" + Html;
                // Приведем строку к виду массива байт
                Console.WriteLine(Str);

                //need set up NuGet System.Text.Encoding.CodePages
                //https://docs.microsoft.com/en-us/dotnet/api/system.text.codepagesencodingprovider?redirectedfrom=MSDN&view=net-5.0#Anchor_4
                //https://stackoverflow.com/questions/50858209/system-notsupportedexception-no-data-is-available-for-encoding-1252/50875725
                Buffer = Encoding.GetEncoding(1251).GetBytes(Str);
                
                //Buffer = Encoding.Convert(Encoding.UTF8, Encoding.GetEncoding(1252), Buffer);
                // Отправим его клиенту                
                _Client.GetStream().Write(Buffer, 0, Buffer.Length);
                // Закроем соединение
                _Client.Close();
            }
            
            private void SendError(int Code)
            {
                // Получаем строку вида "200 OK"
                // HttpStatusCode хранит в себе все статус-коды HTTP/1.1
                string CodeStr = Code.ToString() + " " + ((HttpStatusCode)Code).ToString();
                // Код простой HTML-странички
                string Html = "<html><body><h1>" + CodeStr + "</h1></body></html>";
                // Необходимые заголовки: ответ сервера, тип и длина содержимого. После двух пустых строк - само содержимое
                string Str = "HTTP/1.1 " + CodeStr + "\nContent-type: text/html\nContent-Length:" + Html.Length.ToString() + "\n\n" + Html;
                // Приведем строку к виду массива байт
                byte[] Buffer = Encoding.GetEncoding(1251).GetBytes(Str);
                // Отправим его клиенту
                _Client.GetStream().Write(Buffer, 0, Buffer.Length);
                // Закроем соединение
                _Client.Close();

                
            }

        }


        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");
            
            System.Text.Encoding.RegisterProvider(System.Text.CodePagesEncodingProvider.Instance);
            //string Request = "GET /getWeather_1 HTTP/1.1\r\nHost: 192.168.0.104\r\nUser-Agent: ESP8266HTTPClient\r\nAccept-Encoding: identity;q=1,chunked;q=0.1,*;q=0\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
            //Match ReqMatch = Regex.Match(Request, @"^\w+\s+([^\s\?]+)[^\s]*\s+HTTP/.*|");
            //// Получаем строку запроса
            //string RequestUri = ReqMatch.Groups[1].Value;
            new Server(80);
            Console.ReadKey();
        }
        public static string GetDateWintRemoteServer()
        {
            //string url = "http://api.openweathermap.org/data/2.5/weather?lat=51.6743721&lon=33.9105138&appid=01b822b1051ee607269fc87c780aba06";
            //url = "http://pro.openweathermap.org/data/2.5/forecast/hourly?q=Hlukhiv,UA&mode=xml&appid=01b822b1051ee607269fc87c780aba06";
            //HttpWebRequest httpWebReqest;
            //HttpWebResponse httpWebResponse;
            //HttpWebRequest httpWebReqest = (HttpWebRequest)WebRequest.Create(url);
            //HttpWebResponse httpWebResponse = (HttpWebResponse)httpWebReqest.GetResponse();
            string response;
            //using (StreamReader streamReader = new StreamReader(httpWebResponse.GetResponseStream()))
            //{
            //    response = streamReader.ReadToEnd();
            //}
            //Console.WriteLine(response);
            //int indexStart = response.IndexOf("description")+14, indexStop = response.IndexOf("\"", indexStart), length = indexStop - indexStart;
            //Console.WriteLine("Weather"+response.Substring(indexStart, length));
            //indexStart = response.IndexOf("temp") + 6;
            //indexStop = response.IndexOf("\"", indexStart)-1;
            //length = indexStop - indexStart;
            //string str = response.Substring(indexStart, length);
            //double temperature = Convert.ToDouble(str.Replace('.', ','));
            //Console.WriteLine("Temp: " + Convert.ToString(Math.Round(temperature - 273.15, 2))) ; 
            //indexStart = response.IndexOf("humidity") + 10;
            //indexStop = response.IndexOf("\"", indexStart)-1;
            //length = indexStop - indexStart;
            //Console.WriteLine("Humidity: " + response.Substring(indexStart, length) + "%");
            //url = "https://sinoptik.ua/погода-глухов-303005418";
            //url = "http://worldtimeapi.org/api/timezone/Europe/Kiev";
            //httpWebReqest = (HttpWebRequest)WebRequest.Create(url);
            //httpWebResponse = (HttpWebResponse)httpWebReqest.GetResponse();
            //using (StreamReader streamReader = new StreamReader(httpWebResponse.GetResponseStream()))
            //{
            //    response = streamReader.ReadToEnd();
            //}
            //Console.WriteLine(response);
            response = "{\"abbreviation\":\"EET\",\"client_ip\":\"193.169.125.6\",\"datetime\":\"2021-01-03T14:12:55.780162+02:00\",\"day_of_week\":0,\"day_of_year\":3,\"dst\":false,\"dst_from\":null,\"dst_offset\":0,\"dst_until\":null,\"raw_offset\":7200,\"timezone\":\"Europe/Kiev\",\"unixtime\":1609675975,\"utc_datetime\":\"2021-01-03T12:12:55.780162+00:00\",\"utc_offset\":\"+02:00\",\"week_number\":53}";
            int indexStart = response.IndexOf("datetime") + 11, indexStop = response.IndexOf("\"", indexStart), length = indexStop - indexStart;
            Console.WriteLine("datetime " + response.Substring(indexStart, length));
            string date = response.Substring(indexStart, 10), time = response.Substring(indexStart + 11, 8);
            Console.WriteLine($"Date {date} time {time}");
            return $"Date {date} time {time}";
        }

        public static string GetDateTimePC()
        {
            string date_ = $"{DateTime.Now.Year}-{ControlBitCount(DateTime.Now.Month.ToString(), 2)}" +
                    $"-{ControlBitCount(DateTime.Now.Day.ToString(), 2)} ";
            string time_ = $"time {ControlBitCount(DateTime.Now.Hour.ToString(), 2)}:{ControlBitCount(DateTime.Now.Minute.ToString(), 2)}:" +
                $"{ControlBitCount(DateTime.Now.Second.ToString(), 2)} ";
            string dt_ = $"dt {Convert.ToChar(Convert.ToInt32(DateTime.Now.Year) - 2020 + 30)}" +
                $"{Convert.ToChar(Convert.ToInt32(DateTime.Now.Month) + 30)}" +
                $"{Convert.ToChar(Convert.ToInt32(DateTime.Now.Day) + 30)} ";
            string tm_ = $"tm {Convert.ToChar(Convert.ToInt32(DateTime.Now.Hour)+30)}" +
                $"{Convert.ToChar(Convert.ToInt32(DateTime.Now.Minute) + 30)}" +
                $"{Convert.ToChar(Convert.ToInt32(DateTime.Now.Second) + 30)}";
            return date_ + dt_ + time_  + tm_;
        }
        static DateTime dateTime1 = DateTime.Now;
        static string responseWeather = "";
        public static string GetWeatherSinoptic(E_DAYWEATHER e_DAYWEATHER)
        {
            
            string url = "http://sinoptik.ua/погода-глухов-303005418";
            try
            {
                string response = responseWeather;
                if (String.IsNullOrEmpty(responseWeather) || (dateTime1.AddHours(6) < DateTime.Now))
                {
                    HttpWebRequest httpWebReqest = (HttpWebRequest)WebRequest.Create(url);
                    HttpWebResponse httpWebResponse = (HttpWebResponse)httpWebReqest.GetResponse();

                    using (StreamReader streamReader = new StreamReader(httpWebResponse.GetResponseStream()))
                    {
                        response = streamReader.ReadToEnd();
                    }
                    responseWeather = response;
                    dateTime1 = DateTime.Now;
                }                
                //Console.WriteLine(response);
                //string str1 = date.Substring(8, 2);
                //date = Convert.Convert.ToInt32(str1)+1
                string date = $"{DateTime.Now.Year}-{ControlBitCount(DateTime.Now.Month.ToString(), 2)}" +
                    $"-{ControlBitCount(DateTime.Now.Day.ToString(), 2)}";
                int indexStartToday = response.IndexOf(date, response.IndexOf(date));
                String tempMinToday, tempMaxToday, weatherToday;
                string result = "";
                for (int i = 0; i < 3; i++)
                {
                    int indexStart = response.IndexOf("title", indexStartToday) + 7;
                    int indexStop = response.IndexOf("\"", indexStart);
                    int length = indexStop - indexStart;
                    weatherToday = response.Substring(indexStart, length);
                    indexStartToday = response.IndexOf("temperature", indexStartToday);
                    indexStart = response.IndexOf("<span>", response.IndexOf("min", indexStartToday)) + 6;
                    indexStop = response.IndexOf("&", indexStart);
                    length = indexStop - indexStart;
                    tempMinToday = response.Substring(indexStart, length);
                    indexStart = response.IndexOf("<span>", response.IndexOf("max", indexStartToday)) + 6;
                    indexStop = response.IndexOf("&", indexStart);
                    length = indexStop - indexStart;
                    tempMaxToday = response.Substring(indexStart, length);
                    indexStartToday = response.IndexOf("href", indexStartToday);
                    if(i == (int)e_DAYWEATHER)
                    {
                        DateTime dateTime = DateTime.Now;
                        switch (i)
                        {
                            case 0:
                                result = $"{dateTime.DayOfWeek} {weatherToday} {tempMinToday} {tempMaxToday}";
                                //Console.WriteLine($"Weather today {weatherToday} {tempMinToday} {tempMaxToday}");
                                break;
                            case 1:
                                dateTime = dateTime.AddDays(1);
                                result += $"{dateTime.DayOfWeek} {weatherToday} {tempMinToday} {tempMaxToday}";
                                //Console.WriteLine($"Weather tomorrow {weatherToday} {tempMinToday} {tempMaxToday}");
                                break;
                            case 2:
                                dateTime = dateTime.AddDays(2);
                                result += $"{dateTime.DayOfWeek} {weatherToday} {tempMinToday} {tempMaxToday}";
                                //Console.WriteLine($"Weather tomorrow {weatherToday} {tempMinToday} {tempMaxToday}");
                                break;
                        }
                    }
                    
                }
                return result;
            }
            catch(Exception ex)
            {
                Console.WriteLine($"{ex.Message} {ex.StackTrace}");
                return "501";
            }
        }
        public static string ControlBitCount(string str, int count)
        {
            while (str.Length < count)
            {
                str = str.Insert(0, "0");
            }
            return str;
        }
    }
}
