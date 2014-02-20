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

#if GBC
            descLabel.Text = AppResources.CheatsDescription2;
#endif
        }

        private async void Init()
        {
            object tmp;
            PhoneApplicationService.Current.State.TryGetValue("parameter", out tmp);
            this.romEntry = tmp as ROMDBEntry;
            this.game = romEntry.DisplayName;
            PhoneApplicationService.Current.State.Remove("parameter");
            this.gameNameLabel.Text = game;

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
                else if (tmp.Length == 8)
                {
                    sb.Append(tmp);
                }
                else
                if (tmp.Length == 12)
                {
                    sb.Append(tmp.Substring(0, 8));
                    sb.Append(' ');
                    sb.Append(tmp.Substring(8, 4));
                }
                else if (tmp.Length == 16 || tmp.Length == 17)  //two version of gameshark codes for GBA
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
        
    }
}