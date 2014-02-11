using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using PhoneDirect3DXamlAppInterop.Database;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class PreLinkPage : PhoneApplicationPage
    {
        IEnumerable<ROMDBEntry> romList;

        public PreLinkPage()
        {
            InitializeComponent();

            //create ad control
            if (App.HasAds)
            {
                AdControl adControl = new AdControl();
                ((Grid)(LayoutRoot.Children[0])).Children.Add(adControl);
                adControl.SetValue(Grid.RowProperty, 2);
            }

#if GBC
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
#endif

            romList = ROMDatabase.Current.GetROMList();



            this.firstGamePicker.ItemsSource = romList;
            this.secondGamePicker.ItemsSource = romList;
        }

        private async void linkStartButton_Click(object sender, RoutedEventArgs e)
        {
            ROMDBEntry firstEntry = (ROMDBEntry)firstGamePicker.SelectedItem;
            LoadROMParameter param = await FileHandler.GetROMFileToPlayAsync(firstEntry.FileName);
            firstEntry.LastPlayed = DateTime.Now;
            ROMDatabase.Current.CommitChanges();

            
            

            ROMDBEntry secondEntry = (ROMDBEntry)firstGamePicker.SelectedItem;
            LoadROMParameter param2 = await FileHandler.GetROMFileToPlayAsync(secondEntry.FileName);

            PhoneApplicationService.Current.State["parameter"] = param;
            PhoneApplicationService.Current.State["parameter2"] = param2;

            this.NavigationService.Navigate(new Uri("/LinkedEmulatorPage.xaml", UriKind.Relative));

        }
    }
}