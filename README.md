**How to format and host a Resume on GitHub Pages!**

**Purpose:**

This guide will show the why and how to format a resume in GitHub Flavor Markdown (GFM) using a browser-based text editor, host it on GitHub Pages, and use Jekyll to create a static web page with customizable themes.

This guide will also relate the key principles in technical writing that are defined by Andrew Etter&#39;s book &quot;Modern Technical Writing&quot; with the above instructions.

**Prerequisites:**

- A resume of choice that will be name index.md.
- A README.md file that will provide information that will complements the student&#39;s resume.
- A Markdown editor of choice, this guide will use a browser-based text editor for instruction purpose.
- A git client&#39;s interface, this guide will use git terminal commands.

**Instructions:**

**1. Git, GitHub, and GitHub Pages:**

Git is a free and open-source distributed version control system, which is used primarily in software developments to keep track of all the changes made to the files stored in Git&#39;s project folders called repositories, or repos for short.

GitHub, on the other hand, is a hosting service that helps users manage Git&#39;s repos. It also provides users with a visual comparison between changes made by different commits stored in the repo&#39;s history, thus making it easier to keep track of the changes made to each individual files in each commit.

GitHub Pages is a static site hosting service that works together with Jekyll to generate the static site using the content file (index.md) from username.github.io, where username is the account&#39;s name.

There are Git repos that are hosted on Git&#39;s servers (remote repos), and those that are hosted on local computers (local repos). Remote repos are used as central repos that store the most recent changes made to files. On the other hand, local repos are cloned from remote repos using a git&#39;s client interface, where each user can have their own local copy of the central repo. Every change made to a Git&#39;s repo need to be included in a &#39;git commit&#39; before it can be &#39;git push&#39; to the remote repo. Additionally, all the users who have access to the repo can &#39;git pull&#39; to retrieve the latest change from the remote repo. Git repos and its content&#39;s tracking mechanism are important because they enable concurrent work on the same file and conflicts between different commits can be easily resolved, hence the reason it become popular in the first place. As Andrew Etter mentioned in his book, since Git is a tool for software development, the available features make it trivial to keep a technical document up-to-date and make changes to it easily, such as the resume in this case. For instance, the history tracking&#39;s mechanism in Git makes it possible to store multiple versions of the same files efficiently. It also takes little effort to make changes to the resume, to see what changes were made in the previous commits, or to roll-back and discard all the unwanted changes and start anew with ease. Moreover, a Git repo can be used together with GitHub Pages - a static website hosting service associated with Git – to host a technical document, which is also mentioned by Andrew Etter in his book as a preferable option to other mediums.

The following steps will show how to clone a remote repo to a local repo, add files to the remote repo, and publish the mentioned static website:

- Create a GitHub Account using the following link: [https://github.com/signup](https://github.com/signup).
- Create a public repo with this name&#39;s format: username.github.io, where username is the account&#39;s username
- On the repo&#39;s page, click the green &#39;Code&#39; button, and copy the content to the clipboard.
- Navigate to a folder on the local computer intended to store the local repo using the terminal, then type &#39;git clone address&#39; on it, where address is the content copied from the clipboard.
- When the process finishes running, add the formated index.md and README.md to the directory that store the local repo using File Explorer.
- Switch back to the terminal, run &#39;git add index.md README.md&#39; to add the resume and the README file to the staging area of Git.
- Then, type &#39;git commit -m &quot;Add index.md and README.md&quot;.&#39; to save the change to Git&#39;s content tracking system.
- Type &#39;git push&#39; to push the current version of the local repo to the remote repo.
- If all steps are done correctly, following the url: username.github.io will redirect to the page where the content of index.md is rendered in Markdown Format.

**2. Lightweight Markup Language, Markdown, and GFM Markdown Browser Based Editor:**

Lightweight Markup Language is one of the options for technical writing, one that is strongly recommended by Andrew Etter for good reasons. One of its key characteristics is that it is simple to learn because of the lightweight and human-readable syntax. Since it does not require a lot of effort to learn, it is easy for users to pickup, thus encouraging more contributors to technical documentation sites.

Markdown is one of the most popular markup languages that is used today due to its simple syntax. However, the simplicity also means there is a lack of built-in features that results in many flavors of Markdown, which has inconsistent syntax and features. However, due to is platform-independent nature that are supported by many existing software applications that include GitHub Pages, it is widely popular and will be used to format the resume to be hosted.

To make it easier to format, a GFM editor is used so the format files can be observed from the rendered scene with ease.

**3. Jekyll:**

Jekyll is a static site generator that look for content files in a designated location, which is index.md, or README.md if the former is not found, from the repo username.github.io in this case, render the content in Markdown format, and return a static website ready to be hosted on GitHub Pages.

As remarked by Andrew Etter, a static website is highly preferable to a traditional website because of its simplicity, speed, portability, and security. That is, it takes very little effort to setup a static website, and the user can focus on the contents of the website instead of setting up the website itself. In this case, since GitHub Pages is powered by Jekyll behind the scenes, it makes senses to use Jekyll to customize the website&#39;s theme with it as well.

The following steps detail how to choose a Jekyll theme for the static website:

- On GitHub, navigate to the repo username.github.io and select the &#39;Settings&#39; symbol.
- On the Settings page, look for the &#39;Pages&#39; tab on the left-hand side.
- Navigate to the theme-chooser section and choose a theme.
- Access the url username.github.io to find out what the hosted resume look like in anew theme.

**4. More Resources:**

- Markdown&#39;s tutorial: [https://github.github.com/gfm/](https://github.github.com/gfm/)
- Andrew Etter&#39;s Modern Technical Writing: [https://www.amazon.ca/Modern-Technical-Writing-Introduction-Documentation-ebook/dp/B01A2QL9SS](https://www.amazon.ca/Modern-Technical-Writing-Introduction-Documentation-ebook/dp/B01A2QL9SS)
- Andrew Etter&#39;s guide to install Jekyll locally to access more customizable themes: [https://idratherbewriting.com/documentation-theme-jekyll/mydoc\_install\_jekyll\_on\_windows.html](https://idratherbewriting.com/documentation-theme-jekyll/mydoc_install_jekyll_on_windows.html)

**5. Frequently Asked Questions:**

Q: Is it ok if I want to use another static site generator other than Jekyll, such as Hugo?

A: Absolutely! In fact, there are other site generators out there like Hugo that outperforms Jekyll in terms of loading speed, built-in functionalities, and a thriving support community. Therefore, please feel free to use other static site generator that is specific to your liking and purpose.

Q: Why does the page at username.github.io show the content of README.md but not index.md? Though I uploaded index.md after README.md, does the upload order matter?

A: When GitHub Pages cannot find index.md, it will display the content of README.md instead. You may need to remove README.md first, then upload index.md, and finally upload README.md again afterwards.

**6. Authors and Acknowledgements:**

[https://blog.devmountain.com/git-vs-github-whats-the-difference/](https://blog.devmountain.com/git-vs-github-whats-the-difference/)

[https://forestry.io/blog/hugo-and-jekyll-compared/](https://forestry.io/blog/hugo-and-jekyll-compared/)

[https://github.com/jekyll/jekyll](https://github.com/jekyll/jekyll)