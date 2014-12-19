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
using DucLe.Extensions;
using System.IO;
using SharpGIS;
using System.Threading.Tasks;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class CheatPage : PhoneApplicationPage
    {
        private string game;
        private ROMDBEntry romEntry;
        private List<CheatData> cheatCodes = new List<CheatData>();

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

            


        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            await Init();
            base.OnNavigatedTo(e);
        }

        private async Task Init()
        {
            object tmp;
            PhoneApplicationService.Current.State.TryGetValue("parameter", out tmp);

            if (tmp == null)
            {
                //go back because there is no information about the rom entry
                if (this.NavigationService.CanGoBack)
                    this.NavigationService.GoBack();
                return;
            }

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
            else if (mainPivot.SelectedItem == this.searchPivot
                && (cheatTextStackpanel.Visibility == Visibility.Visible || codeList.Visibility == Visibility.Visible)) //in cheat page and browser windows open
            {
                if (cheatTextStackpanel.Visibility == Visibility.Visible)
                {
                    gameList.Visibility = Visibility.Collapsed;
                    codeList.Visibility = Visibility.Visible;
                    cheatTextStackpanel.Visibility = Visibility.Collapsed;
                }
                else if (codeList.Visibility == Visibility.Visible)
                {
                    gameList.Visibility = Visibility.Visible;
                    codeList.Visibility = Visibility.Collapsed;
                    cheatTextStackpanel.Visibility = Visibility.Collapsed;
                }
                e.Cancel = true;
            }
            else
            {
                //There is no PopUp open and no browser windows open, save then go back
                e.Cancel = true;


                //save the cheat code
                if (this.romEntry != null)
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
            EditCheatControl.PromptText = AppResources.EnterNewCheatText;
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
            if (this.mainPivot.SelectedItem == this.listPivot)
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
            //more information about cheat code format
            //https://answers.yahoo.com/question/index?qid=20060928025359AAQkTkf

            //determine if gameboy or gameboy advance
            bool isGBA = false; //false if gameboy, true if gameboy advance
            string extension = Path.GetExtension(romEntry.FileName).ToLower();

            if (extension == ".gba")
                isGBA = true;

            string[] codeParts = code.Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);
            List<string> codes = new List<string>();

            //string[] codes = new string[codeParts.Length];

            StringBuilder sb = new StringBuilder();
            bool continuedFromLast = false;

            for (int i = 0; i < codeParts.Length; i++)
            {

                string codePart = codeParts[i].ToUpper().Trim().Replace("\t", "");
                string tmp = Regex.Replace(codePart, "[\t\r]", "");
                tmp = Regex.Replace(tmp, "[-]", "");

                if (continuedFromLast == false) //reset the string builder if not continued from last time
                    sb = new StringBuilder();

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
                else if (tmp.Length == 8)  // 12345678, 
                {
                    if (isGBA)  //convert to 12345678 12345678 format for gameboy advance game
                    {
                        if (continuedFromLast) //this is the second part
                        {
                            sb.Append(" " + tmp);
                            continuedFromLast = false;
                        }
                        else
                        {
                            sb.Append(tmp);
                            continuedFromLast = true;
                        }
                    }
                    else
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

                if (continuedFromLast == false)
                    codes.Add(sb.ToString());
            }

            return codes.ToArray() ;
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

            SystemTray.GetProgressIndicator(this).IsIndeterminate = true;
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
                SystemTray.GetProgressIndicator(this).IsIndeterminate = false;
                return;
            }




            List<CheatInfo> PartialMatchList = new List<CheatInfo>();
            List<CheatInfo> CheatInfoList = new List<CheatInfo>();


            //=========get the partial matches
            MatchCollection partialMatches = Regex.Matches(response, "(?<=<div class=\"search_otherresult\">).*?(?=</div>)", RegexOptions.Singleline); //look for the string between <tr class=\"table_sub\"> and </tr>

            for (int i = 0; i < partialMatches.Count; i++)
            {
                CheatInfo cheatInfo = new CheatInfo();

                //get the title
                Match partialMatch = partialMatches[i];
                Match matchTitle = Regex.Match(partialMatch.Value, "(?<=<b>).*?(?=</b>)", RegexOptions.Singleline);
                cheatInfo.Title = matchTitle.Value;

                //===get the platform
                Match matchPlatform = Regex.Match(partialMatch.Value, "(?<=<a href=.*?>).*?(?=</a>)", RegexOptions.Singleline);

                //====add title to list
                PartialMatchList.Add(cheatInfo);

            }



            //get rid of the bold tag
            response = response.Replace("<b>", "");
            response = response.Replace("</b>", "");


            //string test = "hhh\n        <tr class=\"table_head_01\">\n\t<td align=\"left\" valign=\"middle\">Cheats</td><td align=\"left\" valign=\"middle\">Hints</td><td align=\"left\" valign=\"middle\">Q&A</td><td align=\"left\" valign=\"middle\">Walkthroughs</td><td align=\"left\" valign=\"middle\">Screens</td><td align=\"left\" valign=\"middle\">Walls</td><td align=\"left\" valign=\"middle\">Videos</td></tr>";
            //MatchCollection tests = Regex.Matches(test, "(?<=<tr class=\"table_head_01\">).*?(?=</tr>)", RegexOptions.Singleline);

            //=====get the exact match
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
                            cheatInfo.GSLink = "http://www.supercheats.com" + matchhref.Value;
                        }
                        else if (matchtd.Value.Contains("codes2.htm")) //AR
                        {
                            cheatInfo.HasAR = true;
                            Match matchhref = Regex.Match(matchtd.Value, "(?<=<a href=\").*?(?=\")", RegexOptions.Singleline);
                            cheatInfo.ARLink = "http://www.supercheats.com" + matchhref.Value;
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


            


            this.exactMatches.DataContext = CheatInfoList;
            this.partialMatches.DataContext = PartialMatchList;

            gameList.Visibility = Visibility.Visible;
            codeList.Visibility = Visibility.Collapsed;
            cheatTextStackpanel.Visibility = Visibility.Collapsed;

            SystemTray.GetProgressIndicator(this).IsIndeterminate = false;

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





        private void txtSearchString_KeyUp(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == System.Windows.Input.Key.Enter)
            {
                searchButton_Click(null, null);
            }
        }

        private void GSButton_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            CheatInfo cheatInfo = (sender as Button).DataContext as CheatInfo;
            DisplayCheats(cheatInfo.GSLink, 0);

        }

        private void ARButton_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            CheatInfo cheatInfo = (sender as Button).DataContext as CheatInfo;
            DisplayCheats(cheatInfo.ARLink, 1);

        }

        private async void DisplayCheats(string url, int cheatType)
        {
            //cheatType: 0 for GS, 1 for AR

            //navigate to the cheat link and download cheat text
            GZipWebClient webClient = new GZipWebClient();
            string response = null;

            SystemTray.GetProgressIndicator(this).IsIndeterminate = true;

            try
            {
                //USE THIRD-PARTY GZIPWEBCLIENT                
                webClient.Headers[HttpRequestHeader.Accept] = "text/html, application/xhtml+xml, */*";
                webClient.Headers[HttpRequestHeader.Referer] = "http://www.supercheats.com/search.php";
                webClient.Headers[HttpRequestHeader.AcceptLanguage] = "en-US";
                webClient.Headers[HttpRequestHeader.UserAgent] = "Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko";
                webClient.Headers[HttpRequestHeader.AcceptEncoding] = "gzip, deflate";
                webClient.Headers[HttpRequestHeader.Connection] = "Keep-Alive";
                webClient.Headers[HttpRequestHeader.Host] = "www.supercheats.com";

                response = await webClient.DownloadStringTaskAsync(new Uri(url, UriKind.Absolute));


                if (response == null)
                {
                    SystemTray.GetProgressIndicator(this).IsIndeterminate = false;
                    return;
                }


                MatchCollection headers = Regex.Matches(response, "(?<=<h5 class=\"fleft\">).*?(?=</h5>)", RegexOptions.Singleline);


                List<CheatText> CheatTextList = new List<CheatText>();


                for (int i = 0; i < headers.Count; i++)
                {
                    string htmlstring = "<html><head><meta name='viewport' content='width=456, user-scalable=yes' /></head><body>";

                    CheatText cheatText = new CheatText();

                    //get the title
                    if (headers[i].Value == "")
                        cheatText.Title = "No description";
                    else
                        cheatText.Title = headers[i].Value;

                    //get the text content
                    string content = "";
                    if (cheatType == 0) //GS cheats
                    {
                        Match contentMatch = Regex.Match(response, "(?<=<div class=\"body\").*?(?=</div>)", RegexOptions.Singleline);
                        content = contentMatch.Value;
                    }
                    else if (cheatType == 1)
                    {
                        Match contentMatch = Regex.Match(response, "(?<=<div class=\"body\").*?(?=</span></div>)", RegexOptions.Singleline);
                        content = contentMatch.Value;
                    }



                    int index = content.IndexOf(">"); //get the position of the closing bracket
                    content = content.Substring(index + 1); //get rid of the closing bracket

                    //get the text in html format
                    if (cheatType == 0)
                    {
                        htmlstring += "<p style=\"font-size:22px\">" + content + "</p>" + "</body></html>"; //html string does not need to replace <br/> by \n
                        cheatText.TextHtml = htmlstring;
                    }
                    else if (cheatType == 1)
                    {
                        int index2 = content.IndexOf("<div"); //find the end of the title

                        htmlstring += "<p style=\"font-size:22px\">" + content.Substring(0, index2) + "</p>"; //html string does not need to replace <br/> by \n
                        cheatText.TextHtml = htmlstring;
                    }

                    index = response.IndexOf("<div class=\"body\"");
                    response = response.Substring(index + 10); //just any number to make it go pass the <div


                    //get the text in plain text format
                    content = content.Replace("<br/>", "\n");
                    content = content.Replace("<br />", "\n");
                    content = content.Replace("<BR>", "\n");
                    content.Replace("\n\n\n", "\n\n");
                    content = content.Trim();


                    if (cheatType == 1)
                    {
                        //get the link to the full cheat text
                        Match linkMatch = Regex.Match(content, "(?<=<a href=\").*?(?=\")", RegexOptions.Singleline);
                        cheatText.Url = linkMatch.Value;

                        //get the text part
                        int index2 = content.IndexOf("<div"); //find the end of the text

                        if (index2 > 0)
                            content = content.Substring(0, index2 - 1); //there is a \t at the end so get rid of it.
                        else
                            content = "";
                    }

                    //get only the first few lines
                    int count = 0;
                    index = 0;
                    cheatText.Text = "";
                    while (count <= 4 && index != -1)
                    {
                        index = content.IndexOf("\n");

                        if (index != -1)
                        {
                            count++;
                            cheatText.Text += content.Substring(0, index + 1);  //copy the first line to cheatText

                            content = content.Substring(index + 1); //remove the first line
                            content = content.Trim();
                        }
                        else
                        {
                            cheatText.Text += content;
                        }
                    }

                    if (cheatText.Text.Length > 0 && cheatText.Text[cheatText.Text.Length - 1] != '\n')
                        cheatText.Text += "\n";

                    cheatText.Text += "..............";

                    //add to list
                    CheatTextList.Add(cheatText);

                }


                codeList.DataContext = CheatTextList;


                
                gameList.Visibility = Visibility.Collapsed;
                codeList.Visibility = Visibility.Visible;
                cheatTextStackpanel.Visibility = Visibility.Collapsed;

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            SystemTray.GetProgressIndicator(this).IsIndeterminate = false;
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

        private async void codeList_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (codeList.SelectedItem == null)
                return;

            CheatText cheatText = (CheatText)codeList.SelectedItem;
            codeList.SelectedItem = null;


           if (cheatText.Url != null && cheatText.Url != "") //need to go to the link to obtain the code
            {
                GZipWebClient webClient = new GZipWebClient();
                string response = null;
                SystemTray.GetProgressIndicator(this).IsIndeterminate = true;

                try
                {
                    //USE THIRD-PARTY GZIPWEBCLIENT                
                    webClient.Headers[HttpRequestHeader.Accept] = "text/html, application/xhtml+xml, */*";
                    webClient.Headers[HttpRequestHeader.Referer] = "http://www.supercheats.com/search.php";
                    webClient.Headers[HttpRequestHeader.AcceptLanguage] = "en-US";
                    webClient.Headers[HttpRequestHeader.UserAgent] = "Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko";
                    webClient.Headers[HttpRequestHeader.AcceptEncoding] = "gzip, deflate";
                    webClient.Headers[HttpRequestHeader.Connection] = "Keep-Alive";
                    webClient.Headers[HttpRequestHeader.Host] = "www.supercheats.com";

                    response = await webClient.DownloadStringTaskAsync(new Uri(cheatText.Url, UriKind.Absolute));


                    if (response == null)
                    {
                        SystemTray.GetProgressIndicator(this).IsIndeterminate = false;
                        return;
                    }

                    Match contentMatch = Regex.Match(response, "(?<=<div id='sub).*?(?=</div>)", RegexOptions.Singleline);
                    string content = contentMatch.Value;
                    int index = content.IndexOf(">"); //get the position of the closing bracket
                    content = content.Substring(index + 1); //get rid of the closing bracket

                    cheatText.TextHtml += "<p style=\"font-size:22px\">" + content + "</p></body></html>";
                    
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }

                SystemTray.GetProgressIndicator(this).IsIndeterminate = false;

            }

            cheatTextStackpanel.DataContext = cheatText;
            cheatTextBox.NavigateToString(cheatText.TextHtml);

            gameList.Visibility = Visibility.Collapsed;
            codeList.Visibility = Visibility.Collapsed;
            cheatTextStackpanel.Visibility = Visibility.Visible;
        }

        private void TextBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            codeList.Visibility = Visibility.Visible;
            cheatTextStackpanel.Visibility = Visibility.Collapsed;
        }

        private void partialMatches_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            CheatInfo entry = (CheatInfo)partialMatches.SelectedItem;
            partialMatches.SelectedItem = null;

            txtSearchString.Text = entry.Title;

            searchButton_Click(null, null);

           
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

    public class CheatText
    {
        public string Title { get; set; }
        public string Text { get; set; }
        public string TextHtml { get; set; }
        public string Url { get; set; } //for AR code we cannot get the link right away, so need this one
    }
}