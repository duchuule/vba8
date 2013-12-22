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

namespace PhoneDirect3DXamlAppInterop
{
    public partial class CheatPage : PhoneApplicationPage
    {
        private string game;
        private ROMDBEntry romEntry;
        private List<CheatData> cheatCodes = new List<CheatData>();

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

        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);

            base.OnNavigatingFrom(e);
        }

        private void CreateAppBar()
        {
            ApplicationBar = new ApplicationBar();
            ApplicationBar.IsVisible = false;

            var removeButton = new ApplicationBarIconButton(new Uri("/Assets/Icons/delete.png", UriKind.Relative))
            {
                Text = AppResources.DeleteCheatButtonText
            };
            removeButton.Click += removeButton_Click;

            ApplicationBar.Buttons.Add(removeButton);
        }

        void removeButton_Click(object sender, EventArgs e)
        {
            if (this.cheatList.SelectedItem == null)
            {
                MessageBox.Show(AppResources.DeleteCheatNoSelection, AppResources.ErrorCaption, MessageBoxButton.OK);
                return;
            }

            this.cheatCodes.Remove(this.cheatList.SelectedItem as CheatData);
            this.cheatList.SelectedItem = null;
            this.RefreshCheatList();
            FileHandler.SaveCheatCodes(this.romEntry, this.cheatCodes);
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

        private void addButton_Click(object sender, RoutedEventArgs e)
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

#if !GBC
        private string[] GetCodes(string code)
        {
            string[] codeParts = code.Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);
            string[] codes = new string[codeParts.Length];
            for (int i = 0; i < codeParts.Length; i++)
            {
                string codePart = codeParts[i].ToUpper().Trim().Replace("\t", "");
                string tmp = Regex.Replace(codePart, "[\t\r ]", "");
                StringBuilder sb = new StringBuilder();
                if (tmp.Length == 12)
                {
                    sb.Append(tmp.Substring(0, 8));
                    sb.Append(' ');
                    sb.Append(tmp.Substring(8, 4));
                }
                else if (tmp.Length == 16)
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
                try
                {
                    Int64.Parse(tmp, System.Globalization.NumberStyles.HexNumber);
                }
                catch (Exception)
                {
                    messageCallback(AppResources.CheatInvalidFormat);
                    return false;
                }
                if (tmp.Length != 12 && tmp.Length != 16)
                {
                    messageCallback(AppResources.CheatInvalidFormat);
                    return false;
                }
            }
            return true;
        }
#else
        private string[] GetCodes(string code)
        {
            string[] codeParts = code.Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);
            string[] codes = new string[codeParts.Length];
            for (int i = 0; i < codeParts.Length; i++)
            {
                string codePart = codeParts[i].ToUpper().Trim().Replace("\t", "");
                string tmp = Regex.Replace(codePart, "[- ]", "");
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
                string tmp = Regex.Replace(line, "[- ]", "");
                try
                {
                    Int64.Parse(tmp, System.Globalization.NumberStyles.HexNumber);
                }
                catch (Exception)
                {
                    messageCallback(AppResources.CheatInvalidFormat2);
                    return false;
                }
                if (tmp.Length != 6 && tmp.Length != 9 && tmp.Length != 8)
                {
                    messageCallback(AppResources.CheatInvalidFormat2);
                    return false;
                }
            }
            return true;
        }
#endif

        private void cheatEnabledBox_Checked(object sender, RoutedEventArgs e)
        {
            ListBoxItem contextMenuListItem = this.cheatList.ItemContainerGenerator.ContainerFromItem((sender as CheckBox).DataContext) as ListBoxItem;
            CheatData cheatData = contextMenuListItem.DataContext as CheatData;
            cheatData.Enabled = true;
        }

        private void cheatEnabledBox_Unchecked(object sender, RoutedEventArgs e)
        {
            ListBoxItem contextMenuListItem = this.cheatList.ItemContainerGenerator.ContainerFromItem((sender as CheckBox).DataContext) as ListBoxItem;
            CheatData cheatData = contextMenuListItem.DataContext as CheatData;
            cheatData.Enabled = false;
        }
    }
}