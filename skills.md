# Unreal Engine Development Assistant Skill

This is a comprehensive Claude skill for Unreal Engine development assistance, created by dstn2000. Here's what it covers:

## Core Philosophy
The skill emphasizes **zero assumptions** about user projects. It requires discovery-based workflows before offering code suggestions, recognizing that "Every Unreal project is unique in structure, assets, and configuration."

## Key Features

**Pre-Flight Discovery Protocol** includes:
- Locating and analyzing `.uproject` files to extract engine version, plugins, and modules
- Mapping standard project structure (Source/, Content/, Config/ directories)
- Discovering C++ classes and content assets through file system exploration
- Reviewing existing code patterns before providing solutions

**Input System Guidance** covers:
- Enhanced Input System discovery (UE5+)
- Finding Input Actions and Mapping Contexts
- C++ binding templates and patterns
- Navigation of binary `.uasset` files

**Gameplay Ability System (GAS) Support** includes:
- Setup requirements and module dependencies
- Component placement strategies
- Ability granting and activation patterns
- Common GAS implementation patterns

**Plugin-Specific Help**:
- Documentation search strategies
- Source code examination approaches
- Handling experimental/undocumented plugins

## Referenced Documentation
The skill includes detailed reference materials for:
- Enhanced Input System API and patterns
- Complete GAS setup and replication strategies
- Common pitfalls and troubleshooting

**Weekly Installs**: 117 | **GitHub Stars**: 9 | **First Seen**: Jan 24, 2026
