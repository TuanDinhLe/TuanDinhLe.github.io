# **How to host a Resume on GitHub Pages!**

### **Purpose:**

This guide will show you how to host your Markdown-formatted resume on GitHub Pages, use Jekyll to generate a static website and customize your website&#39;s theme. Also, the key principles in technical writing that are defined by Andrew Etter&#39;s book &quot;Modern Technical Writing&quot; will also be related with the above instructions.

**1. Why Markdown?**

Before defining Markdown, it would be helpful to know what a Lightweight Markup Language is. Lightweight Markup Language is one of the most popular options for technical writing, one that is strongly recommended by Andrew Etter. More specifically, it is simple to learn because of the lightweight and human-readable syntax. New users can quickly pick up the language and they can start contributing right away.

Markdown is one of the most popular markup languages that is used today because it has the cleanest syntax compared to the other markup languages. However, it supports a limited number of built-in features that is mitigated by the fact that there are many Markdown flavors. However, different flavors of Markdown have subtle differences users need to be aware of. For our purpose, GitHub Flavored Markdown will be used.


**2. Got it, Markdown is good, but how does GitHub come into the equation?**

Excellent question! Resume is a document that needs to be kept up to date frequently, and Git allows you to do that with ease! It is important to make a distinction between Git, GitHub, and GitHub Pages.

Git is a free and open-source distributed version control system, which is used primarily in software developments to keep track of all the changes made to the files stored in Git&#39;s project folders called repositories, or repos for short.

GitHub, on the other hand, is a hosting service that helps users manage Git&#39;s repos. It also provides users with visual comparisons between changes made by different commits stored in the repo&#39;s history, thus making it easier to keep track of the changes made to each individual file in each commit.

GitHub Pages is a static site hosting service that works together with Jekyll to generate the static site using the content file (index.md) from username.github.io, where username is the account&#39;s name.

As Andrew Etter mentioned in his book, since Git is a tool for software development, the available features make it trivial to keep your resume up-to-date and make changes to it with little effort. For instance, the history tracking&#39;s mechanism in Git makes it easy to make changes to the resume, to see what changes were made in the previous commits, or to roll-back and discard all the unwanted changes and start anew with ease.

**3. Cool, so there is only 1 missing piece, what is Jekyll?**

Jekyll is a static site generator that looks for content files, which is index.md, or README.md if the former is not found, render the content in Markdown format, and return a static website ready to be hosted on GitHub Pages.

As remarked by Andrew Etter, a static website is highly preferable to a traditional website because of its simplicity, speed, portability, and security. That is, it takes very little effort to set up a static website, and the user can focus on the contents of the website instead of setting up the website itself.
****

### **Prerequisites:**

- [A GitHub account.](https://github.com/join)
- [Visual Studio Code.](https://code.visualstudio.com/download)
- Your resume (duh!).
****
### **Instructions:**

#### **Format your resume using Visual Studio Code:**

- Install Visual Studio Code (VS Code) using the link above.
- Open VS Code and create a new File, save, and name it index.md.
- Get your resume ready, and format it in GitHub Flavored Markdown using the tutorials from the More Resources section below.

Note: You can also use a browser-based editor to observe the markdown format changes in real-time!

#### **Create a new Git repository and retrieve its address:**

- Go to your GitHub account and create a new public repo with the following name: repo username.github.io, where username is your account&#39;s name.
- On the repo&#39;s page, click the green &#39;Code&#39; button, and copy the content to the clipboard.

Note: This is the repo&#39;s address that can be used to &#39;clone&#39; it to a local directory (a local repo)) on your computer, allowing you to make changes and publish those changes to the repo on Git (a remote repo).

#### **Clone the repo:**

- Open your terminal in VS Code by clicking on the Terminal Tab at the top.
- Navigate to a directory you want to clone the repo to using the command &#39;cd local\_repo\_directory&#39; on the terminal.
- Type &#39;git clone remote\_repo\_address&#39; on it, where address is the content copied from the clipboard in the previous step.

Note: These steps provide you with a local copy of the remote repo that you can make changes to and publish it.

#### **Add the resume to Git repo and publish it:**

- When finished cloning, copy index.md to the directory that you store the local repo with File Explorer.
- Switch back to the terminal, run &#39;git add index.md&#39; to add the file to the staging area of Git.
- Then, type &#39;git commit -m &quot;Add index.md&quot;.&#39; to save the change to Git&#39;s content tracking system.
- Type &#39;git push&#39; to push the current version of the local repo to the remote repo.

Voila! If all steps are done correctly, following the url: username.github.io will redirect you to your new static website where the content of index.md is rendered nicely in Markdown Format.

#### **Choose a theme for your new website:**

- On GitHub, navigate to the repo username.github.io and select the &#39;Settings&#39; symbol.
- On the Settings page, look for the &#39;Pages&#39; tab on the left-hand side menu.
- Navigate to the theme-chooser section and choose a theme.
- Access the url username.github.io to find out what the hosted resume looks like in your newly chosen theme.

Note: You can always follow the above steps again to change to a new default theme.

#### **Change the title of your website:**

- Using the terminal to navigate to your local repo directory.
- Type &#39;git pull&#39; to get the latest change from the remote repo to get the newly generated _config.yml file. 
- Open \_config.yml, which is your static website theme's config file.
- Add in these 2 extra lines:

	title: [The title of your site] \
	description: [A short description of your site&#39;s purpose]

- Type &#39;git add \_config.yml&#39; to add the file to the staging area of Git.
- Type &#39;git commit &quot;Add title and description to static website&quot;.&#39; to save the change to Git&#39;s content tracking system.
- Type &#39;git push&#39; to push the current version of the local repo to the remote repo.. 

It is helpful to write a good commit message, you will look at it in the future!

#### **Finish!**

And that&#39;s how you host your Resume using GitHub Pages and customize it with Jekyll. Thank you for following this guide till the end!

### **More Resources:**

- [Visual Studio Code&#39;s tutorial.](https://code.visualstudio.com/docs/introvideos/basics)
- [Github Flavor Markdown&#39;s tutorial.](https://github.github.com/gfm/)
- [Git command line&#39;s tutorial.](https://docs.gitlab.com/ee/gitlab-basics/start-using-git.html)
- [Extra Jekyll&#39;s tutorial.](https://idratherbewriting.com/documentation-theme-jekyll/mydoc_install_jekyll_on_windows.html)
- [Andrew Etter&#39;s Modern Technical Writing.](https://www.amazon.ca/Modern-Technical-Writing-Introduction-Documentation-ebook/dp/B01A2QL9SS)

### **Authors and Acknowledgements:**

My thanks are given to Tuan Le - myself for writing this guide, and my fellow team members for assisting me during the peer review session: Michael Bathie, Andy Tan, Faith De Leon, and Koye Fatoki. I also thank Andrew Etter for his amazing book, Modern Technical Writing, that provided the foundation for this document, and Parker Moore for the amazing Slate theme that my static website is currently using.

### **Frequently Asked Questions:**

Q: Is it ok if I want to use another static site generator other than Jekyll, such as Hugo?

A: Absolutely! In fact, there are other site generators out there like Hugo that outperforms Jekyll in terms of loading speed, built-in functionalities, and a thriving support community. Therefore, please feel free to use other static site generators that are specific to your liking and purpose.

Q: Why does the page at username.github.io show the content of README.md but not index.md? Though I uploaded index.md after README.md, does the upload order matter?

A: When GitHub Pages cannot find index.md, it will display the content of README.md instead. You may need to remove README.md first, then upload index.md, and finally upload README.md again afterwards.