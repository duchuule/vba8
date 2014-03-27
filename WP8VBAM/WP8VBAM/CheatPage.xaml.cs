using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using PhoneDirect3DXamlAppInterop.Resources;
using System.Text.RegularExpressions;
using System.Text;
using PhoneDirect3DXamlAppInterop.Database;
using PhoneDirect3DXamlAppComponent;
using System.Windows.Media;
using Telerik.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Net;
using System.Text.RegularExpressions;
using DucLe.Extensions;
using System.IO;
using SharpGIS;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class CheatPage : PhoneApplicationPage
    {
        private string game;
        private ROMDBEntry romEntry;
        private List<CheatData> cheatCodes = new List<CheatData>();
        private bool initdone = false;

        public Popup popupWindow = null;

        public CheatPage()
        {
            InitializeComponent();

            //create ad control
            if (App.HasAds)
            {
                AdControl adControl = new AdControl();
                LayoutRoot.Children.Add(adControl);
                adControl.SetValue(Grid.RowProperty, 1);
            }

            this.CreateAppBar();

            this.Init();


        }

        private async void Init()
        {
            object tmp;
            PhoneApplicationService.Current.State.TryGetValue("parameter", out tmp);
            this.romEntry = tmp as ROMDBEntry;
            this.game = romEntry.DisplayName;
            PhoneApplicationService.Current.State.Remove("parameter");
            this.gameNameLabel.Text = game;
            this.txtSearchString.Text = romEntry.DisplayName;

            try
            {
                this.cheatCodes = await FileHandler.LoadCheatCodes(this.romEntry);
            }
            catch (Exception) { }

            this.RefreshCheatList();


        }

        //protected override async void OnNavigatingFrom(NavigatingCancelEventArgs e)
        //{
        //    await FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);  //this caused crash because the app leave this page before this is done, so UnauthorizedAcessException

        //    base.OnNavigatingFrom(e);
        //}

        protected override async void OnBackKeyPress(System.ComponentModel.CancelEventArgs e)
        {
            //Check if the PopUp window is open
            if (popupWindow != null && popupWindow.IsOpen)
            {
                //Close the PopUp Window
                popupWindow.IsOpen = false;

                //Keep the back button from navigating away from the current page
                e.Cancel = true;
            }
            else
            {
                //There is no PopUp open, save then go back
                e.Cancel = true;


                //save the cheat code
                await FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);

                if (this.NavigationService.CanGoBack)
                    this.NavigationService.GoBack();
            }


            

        }



        

        private void CreateAppBar()
        {
            ApplicationBar = new ApplicationBar();
            ApplicationBar.IsVisible = false;
            ApplicationBar.BackgroundColor = (Color)App.Current.Resources["CustomChromeColor"];
            ApplicationBar.ForegroundColor = (Color)App.Current.Resources["CustomForegroundColor"];


            //var button = new ApplicationBarIconButton(new Uri("/Assets/Icons/cancel.png", UriKind.Relative))
            //{
            //    Text = AppResources.CancelButtonText
            //};
            //button.Click += cancelButton_Click;

            //ApplicationBar.Buttons.Add(button);



            var button = new ApplicationBarIconButton(new Uri("/Assets/Icons/delete.png", UriKind.Relative))
            {
                Text = AppResources.DeleteCheatButtonText
            };
            button.Click += removeButton_Click;

            ApplicationBar.Buttons.Add(button);

            button = new ApplicationBarIconButton(new Uri("/Assets/Icons/edit.png", UriKind.Relative))
            {
                Text = AppResources.EditText
            };
            button.Click += editButton_Click;

            ApplicationBar.Buttons.Add(button);

            //button = new ApplicationBarIconButton(new Uri("/Assets/Icons/check.png", UriKind.Relative))
            //{
            //    Text = AppResources.OKButtonText
            //};
            //button.Click += OKButton_Click;

            //ApplicationBar.Buttons.Add(button);

        }

        private async void editButton_Click(object sender, EventArgs e)
        {
            if (this.cheatList.SelectedItem == null)
            {
                MessageBox.Show(AppResources.CheatNoSelection, AppResources.ErrorCaption, MessageBoxButton.OK);
                return;
            }

            CheatData cheatdata = this.cheatList.SelectedItem as CheatData;


            //disable current page
            this.IsHitTestVisible = false;
            ApplicationBar.IsVisible = false;

            //create new popup instance


            popupWindow = new Popup();
            EditCheatControl.TextToEdit = cheatdata.CheatCode;

            popupWindow.Child = new EditCheatControl();


            popupWindow.VerticalOffset = 0;
            popupWindow.HorizontalOffset = 0;
            popupWindow.IsOpen = true;

            popupWindow.Closed += async (s1, e1) =>
            {
                this.IsHitTestVisible = true;
                ApplicationBar.IsVisible = true;

                if (EditCheatControl.IsOKClicked &&
                    this.CheckCodeFormat(EditCheatControl.TextToEdit,
                    (s) =>
                    {
                        MessageBox.Show(s, AppResources.ErrorCaption, MessageBoxButton.OK);
                    }))
                {
                    cheatdata.CheatCode = EditCheatControl.TextToEdit;
                    RefreshCheatList();
                    await FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);
                }

            };

            
                


        }


        //private async void OKButton_Click(object sender, EventArgs e)
        //{
        //        await FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);




        //        if (this.NavigationService.CanGoBack)
        //            this.NavigationService.GoBack();
        //}

        //private void cancelButton_Click(object sender, EventArgs e)
        //{
        //    if (this.NavigationService.CanGoBack)
        //        this.NavigationService.GoBack();
        //}

        private async void removeButton_Click(object sender, EventArgs e)
        {
            if (this.cheatList.SelectedItem == null)
            {
                MessageBox.Show(AppResources.CheatNoSelection, AppResources.ErrorCaption, MessageBoxButton.OK);
                return;
            }

            this.cheatCodes.Remove(this.cheatList.SelectedItem as CheatData);
            this.cheatList.SelectedItem = null;
            this.RefreshCheatList();
            await FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);
        }

        private void pivot_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (this.pivot.SelectedItem == this.listPivot)
            {
                this.ShowAppBar();
            }
            else
            {
                this.HideAppBar();
            }
        }

        private void ShowAppBar()
        {
            ApplicationBar.IsVisible = true;
        }

        private void HideAppBar()
        {
            ApplicationBar.IsVisible = false;
        }

        private async void  addButton_Click(object sender, RoutedEventArgs e)
        {
            if (!this.CheckCodeFormat(this.cheatCodeBox.Text,
                (s) =>
                {
                    MessageBox.Show(s, AppResources.ErrorCaption, MessageBoxButton.OK);
                }))
            {
                return;
            }
            string[] codes = this.GetCodes(this.cheatCodeBox.Text);
            for (int i = 0; i < codes.Length; i++)
            {
                CheatData cheat = new CheatData()
                {
                    CheatCode = codes[i],
                    Description = this.cheatDescBox.Text,
                    Enabled = true
                };
                this.cheatCodes.Add(cheat);
            }
            RefreshCheatList();
            this.cheatCodeBox.Text = "";
            this.cheatDescBox.Text = "";

            FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);
        }

        private void RefreshCheatList()
        {
            this.cheatList.ItemsSource = null;
            this.cheatList.ItemsSource = this.cheatCodes;
        }


        private string[] GetCodes(string code)
        {
            string[] codeParts = code.Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);
            string[] codes = new string[codeParts.Length];
            for (int i = 0; i < codeParts.Length; i++)
            {

                string codePart = codeParts[i].ToUpper().Trim().Replace("\t", "");
                string tmp = Regex.Replace(codePart, "[\t\r]", "");
                tmp = Regex.Replace(tmp, "[-]", "");
                StringBuilder sb = new StringBuilder();
                if (tmp.Length == 6)
                {
                    sb.Append(tmp.Substring(0, 3));
                    sb.Append('-');
                    sb.Append(tmp.Substring(3, 3));
                }
                else if (tmp.Length == 9)
                {
                    sb.Append(tmp.Substring(0, 3));
                    sb.Append('-');
                    sb.Append(tmp.Substring(3, 3));
                    sb.Append('-');
                    sb.Append(tmp.Substring(6, 3));
                }
                else if (tmp.Length == 8)  // 12345678
                {
                    sb.Append(tmp);
                }
                else if (tmp.Length == 12) //123456781234
                {
                    sb.Append(tmp.Substring(0, 8));
                    sb.Append(' ');
                    sb.Append(tmp.Substring(8, 4));
                }
                else  if (tmp.Length == 13) //12345678 1234
                {
                    sb.Append(tmp);
                }
                else if (tmp.Length == 16 || tmp.Length == 17)  //two version of gameshark codes for GBA, 1234567812345678 OR 12345678 12345678
                {
                    //sb.Append(tmp.Substring(0, 8));
                    //sb.Append(' ');
                    //sb.Append(tmp.Substring(8, 8));
                    sb.Append(tmp);
                }
                codes[i] = sb.ToString();
            }

            return codes;
        }

        private bool CheckCodeFormat(string code, Action<String> messageCallback)
        {
            if (String.IsNullOrWhiteSpace(code))
            {
                messageCallback(AppResources.CheatNoCodeEntered);
                return false;
            }
            string[] codeParts = code.Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < codeParts.Length; i++)
            {
                string line = codeParts[i].ToUpper().Trim().Replace("\t", "");
                string tmp = Regex.Replace(line, "[\t\r ]", "");
                tmp = Regex.Replace(tmp, "[- ]", "");
                try
                {
                    Int64.Parse(tmp, System.Globalization.NumberStyles.HexNumber);
                }
                catch (Exception)
                {
                    messageCallback(AppResources.CheatInvalidFormat);
                    return false;
                }
                if (tmp.Length != 6 && tmp.Length != 9 && tmp.Length != 8 && tmp.Length != 12 && tmp.Length != 16)
                {
                    messageCallback(AppResources.CheatInvalidFormat);
                    return false;
                }
            }
            return true;
        }


        //private async void cheatEnabledBox_Checked(object sender, RoutedEventArgs e)
        //{

        //        ListBoxItem contextMenuListItem = this.cheatList.ItemContainerGenerator.ContainerFromItem((sender as CheckBox).DataContext) as ListBoxItem;
        //        CheatData cheatData = contextMenuListItem.DataContext as CheatData;
        //        cheatData.Enabled = true;


        //}

        //private async void cheatEnabledBox_Unchecked(object sender, RoutedEventArgs e)
        //{
        //        ListBoxItem contextMenuListItem = this.cheatList.ItemContainerGenerator.ContainerFromItem((sender as CheckBox).DataContext) as ListBoxItem;
        //        CheatData cheatData = contextMenuListItem.DataContext as CheatData;
        //        cheatData.Enabled = false;

        //}

        private void cheatEnabledBox_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            bool isChecked = (sender as CheckBox).IsChecked.Value;

            ListBoxItem contextMenuListItem = this.cheatList.ItemContainerGenerator.ContainerFromItem((sender as CheckBox).DataContext) as ListBoxItem;
            CheatData cheatData = contextMenuListItem.DataContext as CheatData;
            cheatData.Enabled = isChecked;
        }

        private async void searchButton_Click(object sender, RoutedEventArgs e)
        {
            string body = "search=" + this.txtSearchString.Text;
            GZipWebClient webClient = new GZipWebClient();
            string response = null;

            try
            {
                //==WINDOWS PHONE DOES NOT SUPPORT GZIP STREAM DECOMPRESSION, SO CANNOT USE HTTPWEBREQUEST

                //// Create a new HttpWebRequest object.
                //HttpWebRequest request = (HttpWebRequest)WebRequest.Create("http://www.supercheats.com/search.php");
                //request.Accept = "text/html, application/xhtml+xml, */*";
                //request.Headers[HttpRequestHeader.Referer] = "http://www.supercheats.com/search.php";
                //request.Headers[HttpRequestHeader.AcceptLanguage] = "en-US";
                //request.ContentType = "application/x-www-form-urlencoded"; 
                //request.UserAgent = "Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko";
                //request.Headers[HttpRequestHeader.AcceptEncoding] = "gzip, deflate";
                //request.Headers[HttpRequestHeader.Connection] = "Keep-Alive";
                //request.Headers[HttpRequestHeader.Host] = "www.supercheats.com";
                //request.Headers[HttpRequestHeader.Pragma] = "no-cache";
                //request.ContentLength = str.Length;
                //// Set the Method property to 'POST' to post data to the URI.
                //request.Method = "POST";
                //// start the asynchronous operation
                //request.BeginGetRequestStream(new AsyncCallback(GetRequestStreamCallback), request);


                //USE THIRD-PARTY GZIPWEBCLIENT INSTEAD
                webClient.Headers[HttpRequestHeader.Accept] = "text/html, application/xhtml+xml, */*";
                webClient.Headers[HttpRequestHeader.Referer] = "http://www.supercheats.com/search.php";
                webClient.Headers[HttpRequestHeader.AcceptLanguage] = "en-US";
                webClient.Headers[HttpRequestHeader.ContentType] = "application/x-www-form-urlencoded";
                webClient.Headers[HttpRequestHeader.UserAgent] = "Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko";
                webClient.Headers[HttpRequestHeader.AcceptEncoding] = "gzip, deflate";
                webClient.Headers[HttpRequestHeader.Connection] = "Keep-Alive";
                webClient.Headers[HttpRequestHeader.Host] = "www.supercheats.com";
                webClient.Headers[HttpRequestHeader.Pragma] = "no-cache";
                webClient.Headers[HttpRequestHeader.ContentLength] = body.Length.ToString();

                response = await webClient.UploadStringTaskAsync(new Uri("http://www.supercheats.com/search.php", UriKind.Absolute), body);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            if (response == null)
            {
                return;
            }

            List<CheatInfo> CheatInfoList = new List<CheatInfo>();

            //get rid of the bold tag
            response = response.Replace("<b>", "");
            response = response.Replace("</b>", "");

            int index = response.IndexOf("Exact Matches");
            response = response.Substring(index);

            //string test = "hhh\n        <tr class=\"table_head_01\">\n\t<td align=\"left\" valign=\"middle\">Cheats</td><td align=\"left\" valign=\"middle\">Hints</td><td align=\"left\" valign=\"middle\">Q&A</td><td align=\"left\" valign=\"middle\">Walkthroughs</td><td align=\"left\" valign=\"middle\">Screens</td><td align=\"left\" valign=\"middle\">Walls</td><td align=\"left\" valign=\"middle\">Videos</td></tr>";
            //MatchCollection tests = Regex.Matches(test, "(?<=<tr class=\"table_head_01\">).*?(?=</tr>)", RegexOptions.Singleline);

            MatchCollection headers = Regex.Matches(response, "(?<=<tr class=\"table_sub\">).*?(?=</tr>)", RegexOptions.Singleline); //look for the string between <tr class=\"table_sub\"> and </tr>

            MatchCollection contents = Regex.Matches(response, "(?<=<tr class=\"table_head_01\">).*?(?=</tr>)"); //look for the string between <tr class=\"table_head_01\"> and </tr>
                                                                                                                //don't use Singleline to avoid 2 unncessearies results


            for (int i = 0; i < headers.Count; i++)
            {
                CheatInfo cheatInfo = new CheatInfo();

                //get the title
                Match matchHeader = headers[i];
                Match matchTitle = Regex.Match(matchHeader.Value, "(?<=<a href=.*?>).*?(?=</a>)", RegexOptions.Singleline);
                cheatInfo.Title = matchTitle.Value;

                //===get the links to cheat code
                Match matchContent = contents[i];
                MatchCollection matchTDs = Regex.Matches(matchContent.Value, "(?<=<td align=center class=\"platform_table\">).*?(?=</td>)", RegexOptions.Singleline);
                
                
                //check each <td>

                cheatInfo.HasAR = false;
                cheatInfo.HasGS = false;

                foreach (Match matchtd in matchTDs)
                {
                    if (matchtd.Value.Contains("YES") )
                    {
                        if (matchtd.Value.Contains("codes.htm")) //GS
                        {
                            cheatInfo.HasGS = true;
                            Match matchhref = Regex.Match(matchtd.Value, "(?<=<a href=\").*?(?=\")", RegexOptions.Singleline);
                            cheatInfo.GSLink = matchhref.Value;
                        }
                        else if (matchtd.Value.Contains("codes2.htm")) //AR
                        {
                            cheatInfo.HasAR = true;
                            Match matchhref = Regex.Match(matchtd.Value, "(?<=<a href=\").*?(?=\")", RegexOptions.Singleline);
                            cheatInfo.ARLink = matchhref.Value;
                        }
                    }
                }

                CheatInfoList.Add(cheatInfo);


            //    if (!value.StartsWith("/roms/"))
            //    {
            //        continue;
            //    }
            //    value = string.Concat("http://m.coolrom.com.au", value);
            //    Uri uri = new Uri(value, UriKind.Absolute);
            //    if (uri.Segments.Count() < 4)
            //    {
            //        continue;
            //    }
            //    string str4 = uri.Segments[2].Substring(0, uri.Segments[2].Length - 1);
            //    string str5 = uri.Segments[3].Substring(0, uri.Segments[3].Length - 1);
            //    string lower = str4.ToLower();
            //    string fileNameWithoutExtension = Path.GetFileNameWithoutExtension(uri.LocalPath);
            //    string str6 = fileNameWithoutExtension.Replace("_", " ");
            //    string str7 = string.Concat(fileNameWithoutExtension, ".zip");

            }

            this.gameList.DataContext = CheatInfoList;


            //try
            //{
            //    webClient.Headers["user-agent"] = "Mozilla/5.0 (compatible; MSIE 10.0; Windows Phone 8.0; Trident/6.0; IEMobile/10.0; ARM; Touch; NOKIA; Lumia 520)";
            //    string str3 = await webClient.DownloadStringTaskAsync(new Uri(string.Concat("http://m.coolrom.com.au/search?q=", str1), UriKind.Absolute));
            //    str2 = str3;
            //}
            //catch
            //{
            //}
            //foreach (Match match in Regex.Matches(str2, "(?<=\\bhref=\")[^\"]*"))  //first look for href=", then look for "
            //                                                                       // \\b got translated to \b which is the boundary anchor
            //                                                                       //use @ if does not want to use double \ in \\b but then we cannot escape "
            //{
            //    string value = match.Value;
            //    if (!value.StartsWith("/roms/"))
            //    {
            //        continue;
            //    }
            //    value = string.Concat("http://m.coolrom.com.au", value);
            //    Uri uri = new Uri(value, UriKind.Absolute);
            //    if (uri.Segments.Count() < 4)
            //    {
            //        continue;
            //    }
            //    string str4 = uri.Segments[2].Substring(0, uri.Segments[2].Length - 1);
            //    string str5 = uri.Segments[3].Substring(0, uri.Segments[3].Length - 1);
            //    string lower = str4.ToLower();
            //    string fileNameWithoutExtension = Path.GetFileNameWithoutExtension(uri.LocalPath);
            //    string str6 = fileNameWithoutExtension.Replace("_", " ");
            //    string str7 = string.Concat(fileNameWithoutExtension, ".zip");
                
            //}
        }

        private static void GetRequestStreamCallback(IAsyncResult asynchronousResult)
        {
            HttpWebRequest request = (HttpWebRequest)asynchronousResult.AsyncState;

            // End the operation
            Stream postStream = request.EndGetRequestStream(asynchronousResult);

            string postData = "search=pokemon";

            // Convert the string into a byte array. 
            byte[] byteArray = Encoding.UTF8.GetBytes(postData);

            // Write to the request stream.
            postStream.Write(byteArray, 0, postData.Length);
            postStream.Close();

            // Start the asynchronous operation to get the response
            request.BeginGetResponse(new AsyncCallback(GetResponseCallback), request);
        }


        private static void GetResponseCallback(IAsyncResult asynchronousResult)
        {
            HttpWebRequest request = (HttpWebRequest)asynchronousResult.AsyncState;

            // End the operation
            HttpWebResponse response = (HttpWebResponse)request.EndGetResponse(asynchronousResult);
            
            //check to see if we need to deflate/gzip response
            //windows phone does not support System.IO.Compression !!!!!!!!!!
            //if (response.Headers["Content-Encoding"].ToLower().Contains("gzip"))
            //    responseStream = new GZipStream(responseStream, CompressionMode.Decompress);

            //else if (response.Headers["Content-Encoding"].ToLower().Contains("deflate"))
            //    responseStream = new DeflateStream(responseStream, CompressionMode.Decompress);  

            Stream streamResponse = response.GetResponseStream();
            StreamReader streamRead = new StreamReader(streamResponse);
            string responseString = streamRead.ReadToEnd();
            // Close the stream object
            streamResponse.Close();
            streamRead.Close();

            // Release the HttpWebResponse
            response.Close();

        }

        private void searchCheatButton_Click(object sender, RoutedEventArgs e)
        {

        }


        
    }  //end class


    public class CheatInfo
    {
        public string Title {get; set;}
        public bool HasAR { get; set; }
        public bool HasGS { get; set; }
        public string ARLink { get; set; }
        public string GSLink { get; set; }
    }
}