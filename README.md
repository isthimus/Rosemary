# Rosemary

A funky additive synth in (very) early development

## Environment Setup Instructions

### Required Software
1. Visual Studio 2022 Community Edition
2. JUCE (latest version)
3. Git with Git LFS

### Installing JUCE
1. Download JUCE from either:
   - GitHub: https://github.com/juce-framework/JUCE/releases/latest
   - Or the official website: https://juce.com/download/

2. Extract the downloaded JUCE folder to a convenient location (e.g., your user home folder)

3. Build the Projucer:
   - Navigate to `JUCE/extras/Projucer/Builds/VisualStudio2022`
   - Open the solution file
   - Build in Release mode
   - The Projucer executable will be created in `JUCE/extras/Projucer/Builds/VisualStudio2022/x64/Release/App`

### First-Time Setup

1. **Git LFS Setup**
```bash
# One-time global setup (per machine)
git lfs install

```

2. **Generate Visual Studio Project**
- Open `Rosemary.jucer` in Projucer
- Click "Save Project and Open in IDE"
- This will generate the Visual Studio 2022 solution

3. **VST3 Development Setup** (Optional)
To enable easy testing of VST3 builds:
(This is a one time setup and requires admin rights - 
after which they arent needed again)
```powershell
# First create the rosemary dev builds directory in VST3 common location
mkdir "C:\Program Files\Common Files\VST3\_Vst3Builds\Rosemary"

# Create the local builds directory in the repo
# Note: Replace {RepoPath} with your actual repository path
mkdir "{RepoPath}\Vst3Builds"

# Create symlink from VST3 location to repo
# Note: Replace {RepoPath} with your actual repository path
mklink /D "C:\Program Files\Common Files\VST3\_Vst3Builds\Rosemary" "{RepoPath}\Vst3Builds"
```

#### Visual Studio Post-Build Step
In Visual Studio, add this to your project's Post-Build Event (Configuration Properties -> Build Events -> Post-Build Event):
```batch
xcopy /E /I /Y "$(TargetPath)" "$(ProjectDir)..\..\..\Vst3Builds\Rosemary.vst3\"
```

Notes:
- The `..\..\..` goes up from the VS project directory to the repo root
- `/E /I /Y` flags ensure:
  - `/E` - Copies directories and subdirectories, including empty ones
  - `/I` - If destination doesn't exist, assumes it's a directory
  - `/Y` - Suppresses prompting to confirm overwriting
- This assumes your VST3 is named "Rosemary.vst3" - adjust as needed

#### Projucer Configuration
Alternatively, set this up directly in the Projucer:
1. Open `Rosemary.jucer`
2. In the Visual Studio (2022) exporter settings:
   - Select "Post-Build Command" under "Configuration"
   - Add this command:
   ```
   xcopy /E /I /Y "$(TargetPath)" "$(ProjectDir)..\..\..\Vst3Builds\Rosemary.vst3\"
   ```
3. Save and regenerate the project

The Projucer will automatically add this to all configurations (Debug/Release) when it generates the Visual Studio solution.

### Building the Project
1. Open `Builds/VisualStudio2022/Rosemary.sln`
2. Select your desired configuration (Debug/Release)
3. Build Solution (F7)

### Development Workflow with Git LFS
- Use git normally! LFS handles tracked files automatically
- Check LFS status: `git lfs status`
- List tracked files: `git lfs ls-files`
- Adding new file types to LFS:
```bash
git lfs track "*.extension"
git add .gitattributes
git commit -m "Track new file type in LFS"
```
