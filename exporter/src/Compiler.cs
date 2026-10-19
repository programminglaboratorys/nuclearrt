using System.Diagnostics;

public class Compiler
{
	public void Compile(BuildType buildType, DirectoryInfo outputPath)
	{
		if (buildType == BuildType.SourceCode) return;

		if (buildType == BuildType.WindowsDebug || buildType == BuildType.WindowsRelease)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "windows"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Running CMake to generate the project files...");
			int cmakeExitCode = RunProcess("cmake", "../..", Path.Combine(outputPath.FullName, "build", "windows"));

			if (cmakeExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"CMake failed with exit code {cmakeExitCode}");
				throw new Exception("CMake failed to generate the project files");
			}

			CTFAK.Utils.Logger.Log($"Building...");
			int buildExitCode = RunProcess("cmake", "--build . --config " + (buildType == BuildType.WindowsDebug ? "Debug" : "Release"), Path.Combine(outputPath.FullName, "build", "windows"));

			if (buildExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildExitCode}");
				throw new Exception("Build failed");
			}

		}
		else if (buildType == BuildType.Web)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "web"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Running emcmake cmake to generate the project files...");
			int cmakeExitCode = RunProcess("cmd.exe", "/c emcmake cmake ../.. -DCMAKE_BUILD_TYPE=Release -G Ninja", Path.Combine(outputPath.FullName, "build", "web"));

			if (cmakeExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"CMake failed with exit code {cmakeExitCode}");
				throw new Exception("CMake failed to generate the project files");
			}

			CTFAK.Utils.Logger.Log($"Building...");
			int buildExitCode = RunProcess("cmd.exe", "/c cmake --build . --config Release", Path.Combine(outputPath.FullName, "build", "web"));

			if (buildExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.Switch)
		{
			string projectDir = ToWslPath(outputPath.FullName);
			string buildDir = $"{projectDir}/build/switch";

			string command = $"bash -ic \"source /etc/profile.d/devkit-env.sh && catnip -T Switch -S '{projectDir}' -B '{buildDir}'\"";

			int wslExitCode = RunProcess("wsl", command);
			if (wslExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"WSL failed with exit code {wslExitCode}");
				throw new Exception("WSL failed to generate the project files");
			}
		}
	}

	static string ToWslPath(string windowsPath)
	{
		string full = Path.GetFullPath(windowsPath).Replace('\\', '/');
		return "/mnt/" + char.ToLowerInvariant(full[0]) + full[2..];
	}

	static int RunProcess(string command, string arguments = "", string workingDirectory = "")
	{
		ProcessStartInfo processInfo = new ProcessStartInfo(command, arguments);
		processInfo.WorkingDirectory = workingDirectory;
		processInfo.CreateNoWindow = true;
		processInfo.UseShellExecute = false;
		processInfo.RedirectStandardOutput = true;
		processInfo.RedirectStandardError = true;
		Process? process = Process.Start(processInfo);

		if (process == null)
		{
			throw new Exception($"Failed to start process {command} {arguments}");
		}

		process.OutputDataReceived += (sender, e) =>
		{
			if (e.Data != null)
			{
				CTFAK.Utils.Logger.Log(e.Data);
			}
		};

		process.ErrorDataReceived += (sender, e) =>
		{
			if (e.Data != null)
			{
				CTFAK.Utils.Logger.Log(e.Data);
			}
		};

		process.BeginOutputReadLine();
		process.BeginErrorReadLine();
		process.WaitForExit();

		return process.ExitCode;
	}
}
