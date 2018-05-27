# Make legacy Cura Engine great again

# What
This is an organic growing Cura Engine repository forked from [Cura 15.04 legacy branch](https://github.com/Ultimaker/CuraEngine/tree/legacy). My short-term goal is to experiment the feature that optimizes path planning with asymmetry traveling salesman solver. The long-term goal is to **make legacy Cura Engine great again** (no political implication here). Your contribution is always welcome.

# Who
I'm a hobbyist/maker/programmer/rebel/pro software freedom advocate. I don't work for Ultimaker company who owns Cura engine. Neither do I own an Ultimaker brand 3D printer.

# Why
I feel upset with the recent change of Cura engine (not the UI, I love its new UI) that always messed up slicing with overly complicated settings and deliver sub par planned path in GCode.

I want to make 3D printing accessible to ordinary, regardless which brand of 3D printer you own.

I like the fact that a profit driven company supports an open source software project. However, I'm **strongly** against the idea that an open source software project should be dictated by one single interest party, whose sole interest is to promote their hardware sales. So it is a different matter between authoritarian ownership vs providing financial/human resource support. Because there are obvious ethical problem and interest conflicts between repository owner and outside contributor.

# How
If you feel the same way as I did, you are welcome to join with me.

If you are a non-techie 3D printer like the most, you are welcome to try on this Cura Engine. I will post a release under Mac and Linux for the latest OS.

If you are a programmer who wants to contribute your brain power, I'm open to take a look of your PR. But I want to be clear that I will do it in the following manner:

I will have two branch in public: stable and dev branch.

The stable branch is master branch. If your PR is related to support new hardware setting like dual extruders or a critical bug fix or performance improvement, chances are you are in this category. I will merge it into stable branch ASAP.

The dev branch is for experiment purpose. All dev branch should start with prefix `dev-`. If you require a new settings in profile, chances are you are in dev branch. Things like changing path planning for one particular STL model belongs to this category. I will accept your PR as a separate branch for experimental purpose. After an extended long period of test and usage, I will consider merging it to the stable branch.

# Progress
Several new things I have done so far:
- Shared my 15.04 source code notes and 15.04 class UML in this wiki.
- Enhanced logging with G3LOG
- Enabled CMake build

# Future
Several things I will do next:
- Enable CI like Travis
- Improve path planning
- Promote this legacy repository to non UM brand hackers
- Make legacy Cura Engine great again!

Happy hacking!
Ricky
