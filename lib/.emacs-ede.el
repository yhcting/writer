(set 'yhc.topdir (concat (getenv "PWD") "/"))
(set 'yhc.target "writer")
(setenv "GTAGSROOT" yhc.topdir)
(setenv "GTAGSDBPATH" yhc.topdir)

(load-file "~/.emacs-cedet.el")

(defun yhc.compile.debug ()
    "Using make file in top directory"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "cd " yhc.topdir
                          "; make"))
        ;(message cmd)))
        (compile cmd)))

(defun yhc.compile.release ()
    "Using make file in top directory"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "cd " yhc.topdir "../; " (getenv "NDKROOT") "ndk-build"))
        ;(message cmd)))
        (compile cmd)))

(defun yhc.gdb ()
    "gdb wrapper"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "gdb --annotate=3 " yhc.topdir yhc.target))
        ;(message cmd)))
        (gdb cmd)))

;; override ede's key map for compile...
(global-set-key [f9] 'yhc.compile.debug)
(global-set-key [f10] 'yhc.compile.release)
(global-set-key [f8] 'yhc.gdb)

(ede-cpp-root-project (concat yhc.target "-root")
                :name (concat yhc.target "-root")
                :file (concat yhc.topdir "/Makefile")
                :include-path '("."
                               )
                :system-include-path '("/usr/include")
                :spp-table '() )
