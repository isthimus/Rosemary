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
