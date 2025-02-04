using System.IO;

[module: Sharpmake.Include("../atrium/engine/sharpmake.cs")]

[Sharpmake.Generate]
public class ChartGame_Executable : Atrium.ExecutableProject
{
    public ChartGame_Executable()
    {
        Name = "Chart game executable";
        SourceRootPath = "[project.SharpmakeCsPath]/code/";
    }

    public override void ConfigureAll(Sharpmake.Project.Configuration conf, Sharpmake.Target target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "Executables";
        conf.ProjectPath = "[project.SharpmakeCsPath]/code/";
        
        conf.AddPrivateDependency<Atrium.Engine>(target);

        conf.VcxprojUserFile = new Sharpmake.Project.Configuration.VcxprojUserFileSettings();
        conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = "$(OutputPath)../data/";
    }
}

[Sharpmake.Generate]
public class ChartGameSolution : Atrium.Solution
{
    public ChartGameSolution()
    {
        Name = "Chart game";
    }

    public override void ConfigureAll(Sharpmake.Solution.Configuration conf, Sharpmake.Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionPath = "[solution.SharpmakeCsPath]";

        conf.AddProject<ChartGame_Executable>(target);
    }
}

public static class Main
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Sharpmake.Arguments arguments)
    {
        FileInfo fileInfo = Sharpmake.Util.GetCurrentSharpmakeFileInfo();

        Atrium.Configuration.SolutionDirectory = fileInfo.DirectoryName;
        Atrium.Configuration.BuildDirectory = Path.Combine(
            fileInfo.DirectoryName,
            "build"
        );
        
        arguments.Generate<ChartGameSolution>();
    }
}