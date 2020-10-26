using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Windows.Threading;
using System.Xml.Linq;
using System.Xml;

using System.Net;
using System.Runtime.Serialization;

namespace Puertos_COM
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        FlowDocument mcFlowDoc = new FlowDocument();
        Paragraph para = new Paragraph();
        SerialPort serial = new SerialPort();
        string recieved_data;
        string path = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) + "\\Test_Tiempos.xml";
        string datos = "";
        //bool ultrasonic_estado = false;
        //string link = "http://fourdreams.com.mx/proyectos/down_hill/index.php/api/demo/inserta_participacion?";
        string link = "http://localhost/down_hill/cib3/index.php/api/demo/";
        public bool IsClosed { get; private set; }


        public MainWindow()
        {
            InitializeComponent();
            SerialData.KeyDown += new KeyEventHandler(presionaEnter);

            Connect_btn.Content = "Conectar";
            Connect_btn.Background = Brushes.GreenYellow;
            WriteXML("", "");

            Enviar_sI.IsEnabled = false;
            Enviar_sF.IsEnabled = false;
            f_hora.IsEnabled = false;
            Ultrasonico1.IsEnabled = false;
            Ultrasonico2.IsEnabled = false;
            CheckInicio.IsEnabled = false;
            CheckFinal.IsEnabled = false;
            Limpia.IsEnabled = false;
            //jsonRest("http://fourdreams.com.mx/proyectos/down_hill/index.php/api/demo/inserta_participacion?hora_salida=8:00:00&hora_llegada=11:01:55&tiempo_total=0:1:55:55");
        }

        #region conecta
        private void Connect_Comms(object sender, RoutedEventArgs e)
        {
            string boton = Connect_btn.Content.ToString();
            if (boton == "Conectar")
            {
                if (Baud_Rates.SelectedIndex > -1 && Comm_Port_Names.SelectedIndex > -1)
                {
                    try
                    {
                        //Sets up serial port
                        serial.PortName = Comm_Port_Names.Text;
                        serial.BaudRate = Convert.ToInt32(Baud_Rates.Text);
                        serial.Handshake = System.IO.Ports.Handshake.None;
                        serial.Parity = Parity.None;
                        serial.DataBits = 8;
                        serial.StopBits = StopBits.Two;
                        serial.ReadTimeout = 200;
                        serial.WriteTimeout = 50;
                        serial.Open();

                        //Sets button State and Creates function call on data recieved
                        Connect_btn.Content = "Desconectar";
                        Connect_btn.Background = Brushes.Red;
                        serial.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(Recieve);

                        Enviar_sI.IsEnabled = true;
                        Enviar_sF.IsEnabled = true;
                        f_hora.IsEnabled = true;
                        Ultrasonico1.IsEnabled = true;
                        Ultrasonico2.IsEnabled = true;
                        CheckInicio.IsEnabled = true;
                        CheckFinal.IsEnabled = true;
                        Limpia.IsEnabled = true;
                    }
                    catch
                    {
                        MessageBox.Show("No es posible abrir puerto seleccionado, no existe o está ocupado.");
                    }
                }
                else
                {
                    MessageBox.Show("Selecciona - Puerto COM - y - Baud rate -.");
                }


            }
            else
            {
                try // just in case serial port is not open could also be acheved using if(serial.IsOpen)
                {
                    serial.Close();
                    Connect_btn.Content = "Conectar";
                    Connect_btn.Background = Brushes.GreenYellow;

                    Enviar_sI.IsEnabled = false;
                    Enviar_sF.IsEnabled = false;
                    f_hora.IsEnabled = false;
                    Ultrasonico1.IsEnabled = false;
                    Ultrasonico2.IsEnabled = false;
                    CheckInicio.IsEnabled = false;
                    CheckFinal.IsEnabled = false;
                    Limpia.IsEnabled = false;
                }
                catch
                {
                }
            }
        }
        private void Ajusta_fechaHora(object sender, RoutedEventArgs e)
        {
            DateTime fechaLocal = DateTime.Now;
            String diaSemana = "";
            switch (fechaLocal.DayOfWeek.ToString())
            {
                case "Sunday":
                    diaSemana = "1";
                    break;
                case "Monday":
                    diaSemana = "2";
                    break;
                case "Tuesday":
                    diaSemana = "3";
                    break;
                case "Wednesday":
                    diaSemana = "4";
                    break;
                case "Thursday":
                    diaSemana = "5";
                    break;
                case "Friday":
                    diaSemana = "6";
                    break;
                case "Saturday":
                    diaSemana = "7";
                    break;
            }
            String cadenazo = fechaLocal.Second.ToString() + 's' + fechaLocal.Minute.ToString() + 'm' + fechaLocal.Hour.ToString() + 'h' + diaSemana + 'w' + fechaLocal.Day.ToString() + 'd' + fechaLocal.Month.ToString() + 'M' + fechaLocal.Year.ToString().Substring(fechaLocal.Year.ToString().Length - 2) + 'y';
            SerialCmdSend(cadenazo);
            Console.WriteLine(cadenazo);
        }
        #endregion
        #region Xml
        public void WriteXML(string T_inicio, string T_final)
        {
            DateTime fechaLocal = DateTime.Now;
            if (File.Exists(path))
            {
                XmlDocument xDoc = new XmlDocument();
                xDoc.Load(path);

                if (T_final == "" && T_inicio != "")
                {
                    XmlElement root = xDoc.DocumentElement;
                    XmlNode node = root.SelectSingleNode("//timeInicio");
                    root = xDoc.CreateElement("Inicio");
                    root.InnerText = T_inicio;
                    root.SetAttribute("horaPC", fechaLocal.Hour + ":" + fechaLocal.Minute + ":" + fechaLocal.Second + "." + fechaLocal.Millisecond);
                    node.AppendChild(root);
                    xDoc.Save(path);
                }
                if (T_inicio == "" && T_final != "")
                {
                    XmlElement root = xDoc.DocumentElement;
                    XmlNode node = root.SelectSingleNode("//timeFinal");
                    root = xDoc.CreateElement("Final");
                    root.InnerText = T_final;
                    root.SetAttribute("horaPC", fechaLocal.Hour + ":" + fechaLocal.Minute + ":" + fechaLocal.Second + "." + fechaLocal.Millisecond);
                    node.AppendChild(root);
                    xDoc.Save(path);
                }
            }
            else
            {
                using (FileStream fileStream = new FileStream(path, FileMode.Create))
                using (StreamWriter sw = new StreamWriter(fileStream))
                using (XmlTextWriter xmlWriter = new XmlTextWriter(sw))
                {
                    xmlWriter.Formatting = Formatting.Indented;
                    xmlWriter.Indentation = 4;

                    xmlWriter.WriteStartDocument();
                    xmlWriter.WriteComment("Archivo creado por la aplicación de tiempos");
                    xmlWriter.WriteStartElement("Tiempos");
                    xmlWriter.WriteAttributeString("Fecha", fechaLocal.ToString());

                    xmlWriter.WriteStartElement("timeInicio");
                    xmlWriter.WriteEndElement();

                    xmlWriter.WriteStartElement("timeFinal");
                    xmlWriter.WriteEndElement();

                    xmlWriter.WriteEndElement();
                    xmlWriter.Flush();
                    xmlWriter.WriteEndDocument();

                }
            }
        }
        #endregion
        #region Enviando

        private void Send_Data_I(object sender, RoutedEventArgs e)
        {
            if (String.IsNullOrEmpty(SerialData.Text)) MessageBox.Show("No hay información para enviar.", "Falta información");
            else
            {
                SerialCmdSend(SerialData.Text + "?");
                SerialData.Text = "";
            }

        }
        private void Send_Data_F(object sender, RoutedEventArgs e)
        {
            if (String.IsNullOrEmpty(SerialData.Text)) MessageBox.Show("No hay información para enviar.", "Falta información");
            else
            {
                SerialCmdSend(SerialData.Text + "z");
                SerialData.Text = "";
            }
        }
        private void presionaEnter(object sender, KeyEventArgs e)
        {
            if (!((e.Key >= Key.D0 && e.Key <= Key.D9) || (e.Key >= Key.NumPad0 && e.Key <= Key.NumPad9)|| e.Key == Key.Delete))
            {
                MessageBox.Show("Solo se permiten numeros", "Advertencia");
                e.Handled = true;
                return;
            }
            /*if (e.Key == Key.Enter)
            {
                if (String.IsNullOrEmpty(SerialData.Text)) MessageBox.Show("No hay información para enviar.", "Falta información");
                else
                {
                    SerialCmdSend(SerialData.Text);
                    SerialData.Text = "";
                }
            }*/
        }
        public void SerialCmdSend(string data)
        {
            if (serial.IsOpen)
            {
                try
                {
                    // Send the binary data out the port
                    byte[] hexstring = Encoding.ASCII.GetBytes(data);
                    //There is a intermitant problem that I came across
                    //If I write more than one byte in succesion without a 
                    //delay the PIC i'm communicating with will Crash
                    //I expect this id due to PC timing issues ad they are
                    //not directley connected to the COM port the solution
                    //Is a ver small 1 millisecound delay between chracters
                    foreach (byte hexval in hexstring)
                    {
                        byte[] _hexval = new byte[] { hexval }; // need to convert byte to byte[] to write
                        serial.Write(_hexval, 0, 1);
                        Thread.Sleep(1);
                    }
                }
                catch (Exception ex)
                {
                    para.Inlines.Add("Error al enviar" + data + "\n" + ex + "\n");
                    mcFlowDoc.Blocks.Add(para);
                    Commdata.Document = mcFlowDoc;
                }
            }
            else
            {
            }
        }
        #endregion
        #region Recieving

        private delegate void UpdateUiTextDelegate(string text);
        private void Recieve(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            // Collecting the characters received to our 'buffer' (string).
            recieved_data = serial.ReadExisting();
            Dispatcher.Invoke(DispatcherPriority.Send, new UpdateUiTextDelegate(WriteData), recieved_data);

            datos += recieved_data;
            //if (datos.Contains('/'))
            //{
            //datos = datos.Replace("/", "");
            if (datos.Contains("<i>"))
            {
                DateTime fechandoi = DateTime.Now;
                datos = datos.Replace("<i>", "");
                datos = datos.Replace("\r\n", string.Empty);
                jsonRest(link + "salida?hora=" + datos + "." + fechandoi.Millisecond + "&hit=1");
                WriteXML(datos,"");
                datos = "";
            }
            if (datos.Contains("<f>"))
            {
                DateTime fechandof = DateTime.Now;
                datos = datos.Replace("<f>", "");
                datos = datos.Replace("\r\n", string.Empty);
                jsonRest(link + "llegada?hora=" + datos + "." + fechandof.Millisecond + "&hit=1");
                WriteXML("", datos);
                datos = "";
            }
            //Console.WriteLine(datos);
            if (datos.Contains('.')) datos = "";
            //}
        }
        private void WriteData(string text)
        {
            text = text.Replace("/", "\n");
            para.Inlines.Add(text);
            mcFlowDoc.Blocks.Add(para);
            Commdata.Document = mcFlowDoc;
            Commdata.ScrollToEnd();
        }
        private void Limpia_richTexBox(object sender, RoutedEventArgs e)
        {
            Commdata.Document.Blocks.Clear();
            para.Inlines.Clear();
            mcFlowDoc.Blocks.Clear();

        }
        #endregion
        #region Form Controls
        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            IsClosed = true;
            if (serial.IsOpen) serial.Close();
        }
        #endregion
        private void Sensor_ultrasonico1(object sender, RoutedEventArgs e)
        {
            SerialCmdSend("e");
        }
        private void Sensor_ultrasonico2(object sender, RoutedEventArgs e)
        {
            SerialCmdSend("i");
        }
        private void checkInicio(object sender, RoutedEventArgs e)
        {
            SerialCmdSend(":");
        }
        private void checkFinal(object sender, RoutedEventArgs e)
        {
            SerialCmdSend(".");
        }

        private void jsonRest(string url)
        {
            try
            {
                Task.Run(() =>
                {
                    WebRequest request = WebRequest.Create(url);
                    var response = request.GetResponse();

                    //Do something with response if needed.
                });
            }
            catch
            {
                Console.WriteLine(">> Error enviando datos a aplicación...");
            }

            //var cliente = new WebClient();
            //var contenido = cliente.DownloadData(url);
            //String data_site = cliente.DownloadString(url);

            //var contenido = cliente.DownloadData(url);
            //String data_site = cliente.DownloadString(url);
            
            //System.IO.File.WriteAllText(@"C:\123.txt", data_site);
        }
    }
}
