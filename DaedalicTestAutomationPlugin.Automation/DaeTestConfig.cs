using Gauntlet;
using System.Collections.Generic;

namespace DaedalicTestAutomationPlugin.Automation
{
    public class DaeTestConfig : EpicGame.EpicGameTestConfig
    {
        /// <summary>
		/// Where to write a JUnit XML report to.
		/// </summary>
		[AutoParam]
        public string JUnitReportPath;

        /// <summary>
        /// Where to write test reports to.
        /// </summary>
        [AutoParam]
        public string ReportPath;

        /// <summary>
        /// Which single test to run, instead of all available tests.
        /// </summary>
        [AutoParam]
        public string TestName;

        /// <summary>
        /// Which tests with given tags to run, instead of all available tests.
        /// TestTags takes an argument in the format: Tag1;Tag2;Tag3
        /// If a test has at least one of the required tags, the test will be run.
        /// </summary>
        [AutoParam]
        public string TestTags;

        /// <summary>
        /// Run all tests whose priority is at least as high as the TestPriority.
        /// </summary>
        [AutoParam]
        public string TestPriority;

        public override void ApplyToConfig(UnrealAppConfig AppConfig, UnrealSessionRole ConfigRole, IEnumerable<UnrealSessionRole> OtherRoles)
        {
            base.ApplyToConfig(AppConfig, ConfigRole, OtherRoles);

            if (!string.IsNullOrEmpty(JUnitReportPath))
            {
                AppConfig.CommandLine += $" -JUnitReportPath=\"{JUnitReportPath}\"";
            }

            if (!string.IsNullOrEmpty(ReportPath))
            {
                AppConfig.CommandLine += $" -ReportPath=\"{ReportPath}\"";
            }

            if (!string.IsNullOrEmpty(TestName))
            {
                AppConfig.CommandLine += $" -TestName=\"{TestName}\"";
            }

            if (!string.IsNullOrEmpty(TestTags))
            {
                AppConfig.CommandLine += $" -TestTags=\"{TestTags}\"";
            }

            if (!string.IsNullOrEmpty(TestPriority))
            {
                AppConfig.CommandLine += $" -TestPriority=\"{TestPriority}\"";
            }
        }
    }
}
