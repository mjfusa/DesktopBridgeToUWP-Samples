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

namespace MainApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            var localFolder = Windows.Storage.ApplicationData.Current.LocalFolder.Path;
            var bFileExists = System.IO.File.Exists(localFolder + "\\launcher.out");
            string[] preLauncherResult = new string[5];
            if (bFileExists) {
                preLauncherResult = System.IO.File.ReadAllLines(localFolder + "\\launcher.out");
                if (preLauncherResult[0] == "0")
                {
                    txtLicense.Text = $"Prelauncher ran successfully with exit code {preLauncherResult[0]}";
                }
                else
                {
                    txtLicense.Text = $"Prelauncher ran and returned exit code {preLauncherResult[0]}";
                }
                return;
            }
            txtLicense.Text = $"Prelauncher did not run.";
        }
    }
}
