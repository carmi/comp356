if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <S-Tab> =BackwardsSnippet()
imap <silent> <Plug>IMAP_JumpBack =IMAP_Jumpfunc('b', 0)
imap <silent> <Plug>IMAP_JumpForward =IMAP_Jumpfunc('', 0)
map! <S-Insert> <MiddleMouse>
map  h
snoremap <silent> 	 i<Right>=TriggerSnippet()
nnoremap 	 :call InsertChar#insert(v:count1)
vmap <NL> <Plug>IMAP_JumpForward
nmap <NL> <Plug>IMAP_JumpForward
omap <NL> j
map  k
map  l
snoremap  b<BS>
map   /
vnoremap <silent> # :call VisualSearch('b')
vnoremap $e `>a"`<i"
vnoremap $q `>a'`<i'
vnoremap $$ `>a"`<i"
vnoremap $3 `>a}`<i{
vnoremap $2 `>a]`<i[
vnoremap $1 `>a)`<i(
snoremap % b<BS>%
snoremap ' b<BS>'
vnoremap <silent> * :call VisualSearch('f')
map ,T <Plug>TaskList
nmap <silent> ,t :CommandT
map ,d /:::endworkOistopped <F7>
map ,r /:::endwork2Oistarted <F7>
map ,a :e! /media/truecrypt1/data/notes-all.txt
nmap ,h :set hls!
nmap ,L :match
nmap ,l :match ErrorMsg '\%>80v.\+'
map ,M :call ToggleMaxWins()
noremap ,m mmHmt:%s///ge'tzt'm
map ,s? z=
map ,sa zg
map ,sp [s
map ,sn ]s
map ,ss :setlocal spell!
map ,p :cp
map ,n :cn
map ,cc :botright cope
map ,cd :cd %:p:h
map ,tm :tabmove 
map ,tc :tabclose
map ,te :tabedit 
map ,tn :tabnew %
map ,ba :1,300 bd
map ,bd :Bclose
map <silent> , :noh
map ,g :vimgrep // **/*<Left><Left><Left><Left><Left><Left>
map ,t8 :setlocal shiftwidth=4
map ,t4 :setlocal shiftwidth=4
map ,t2 :setlocal shiftwidth=2
map ,F zC
map ,f zO
map ,q :clo 
map ,u :TlistToggle
map ,N :NERDTreeToggle
map ,e :e! ~/.vimrc
nmap ,w :w
map 0 ^
cmap ½ $
imap ½ $
xmap S <Plug>VSurround
snoremap U b<BS>U
snoremap \ b<BS>\
snoremap ^ b<BS>^
snoremap ` b<BS>`
nmap cs <Plug>Csurround
nmap ds <Plug>Dsurround
nmap gx <Plug>NetrwBrowseX
xmap gS <Plug>VgSurround
vnoremap <silent> gv :call VisualSearch('gv')
xmap s <Plug>Vsurround
nmap ySS <Plug>YSsurround
nmap ySs <Plug>YSsurround
nmap yss <Plug>Yssurround
nmap yS <Plug>YSurround
nmap ys <Plug>Ysurround
snoremap <Left> bi
snoremap <Right> a
snoremap <BS> b<BS>
snoremap <silent> <S-Tab> i<Right>=BackwardsSnippet()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
vmap <silent> <Plug>IMAP_JumpBack `<i=IMAP_Jumpfunc('b', 0)
vmap <silent> <Plug>IMAP_JumpForward i=IMAP_Jumpfunc('', 0)
vmap <silent> <Plug>IMAP_DeleteAndJumpBack "_<Del>i=IMAP_Jumpfunc('b', 0)
vmap <silent> <Plug>IMAP_DeleteAndJumpForward "_<Del>i=IMAP_Jumpfunc('', 0)
nmap <silent> <Plug>IMAP_JumpBack i=IMAP_Jumpfunc('b', 0)
nmap <silent> <Plug>IMAP_JumpForward i=IMAP_Jumpfunc('', 0)
map <F7> a=strftime("%Y%m%d-%H%M%S - %A")
map <F8> a=strftime("<>%Y%m%d - %A")
map <F9> a=strftime("<>%Y%m%d-%H%M%S - time check")
nmap <Left> :bp
xmap <Left> :bp
omap <Left> :bp
nmap <Right> :bn
xmap <Right> :bn
omap <Right> :bn
map <C-Space> ?
map <F5> :TlistAddFilesRecursive . *.py
map <F4> :!/usr/bin/ctags-exuberant -R .
map <S-Insert> <MiddleMouse>
cnoremap  <Home>
cnoremap  <End>
imap S <Plug>ISurround
imap s <Plug>Isurround
inoremap <silent> 	 =TriggerSnippet()
imap <NL> <Plug>IMAP_JumpForward
cnoremap  
cnoremap  <Down>
cnoremap  <Up>
inoremap <silent> 	 =ShowAvailableSnips()
imap  <Plug>Isurround
inoremap $e ""i
inoremap $q ''i
inoremap $4 {o}O
inoremap $3 {}i
inoremap $2 []i
inoremap $1 ()i
cnoremap $q eDeleteTillSlash()
cnoremap $c e eCurrentFileDir("e")
cnoremap $j e ./
cnoremap $d e ~/Desktop/
cnoremap $h e ~/
map! ;v ü
map! ;u û
map! ;t ù
imap ;q «  »hi
map! ;p ö
map! ;o ô
map! ;j ï
map! ;i î
map! ;g é
map! ;f ë
map! ;e ê
map! ;d è
map! ;c ç
map! ;b ä
map! ;a â
map! ;z à
vmap ½ $
nmap ½ $
omap ½ $
vmap ë :m'<-2`>my`<mzgv`yo`z
vmap ê :m'>+`<my`>mzgv`yo`z
nmap ë mz:m-2`z
nmap ê mz:m+`z
imap jj 
iabbr xdate =strftime("%d/%m/%y %H:%M:%S")
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set autoread
set backspace=eol,start,indent
set backupdir=~/.vim_backups//
set directory=~/.vim_backups//
set display=lastline
set expandtab
set fileencodings=ucs-bom,utf-8,default,latin1
set grepprg=/bin/grep\ -nH\ $*
set guioptions=aegit
set guitablabel=%{GuiTabLabel()}
set helplang=en
set hidden
set hlsearch
set ignorecase
set incsearch
set laststatus=2
set listchars=tab:���\ ,trail:·,eol:¬
set matchtime=2
set mouse=a
set omnifunc=ccomplete#Complete
set printoptions=paper:letter
set ruler
set runtimepath=~/.vim,/var/lib/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim72,/usr/share/vim/vimfiles/after,/var/lib/vim/addons/after,~/.vim/after
set scrolloff=3
set shiftwidth=4
set showbreak=↪\ 
set showmatch
set showtabline=0
set smartcase
set smartindent
set smarttab
set softtabstop=4
set statusline=%<%f\ %h%w%m%r%y%=B:%o\ F:%{foldlevel('.')}\ L:%l/%L\ (%p%%)\ C:%c%V\ 
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc,.pyc,.pyo
set switchbuf=usetab
set termencoding=utf-8
set timeoutlen=250
set wildmenu
set wildmode=longest,full
" vim: set ft=vim :
