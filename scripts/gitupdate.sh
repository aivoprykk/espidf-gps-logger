[ -n "$1" ] && cmd=$1;
[ -n "$G"] && do_github="1";
[ "$cmd" = "" ] && { echo 'usage: '"$0"' <pull|push>' && exit; }
 echo "update git"
 for branch in dev master; do
  for i in components/*; do 
	 echo "$i"; echo "$i"|grep lvgl || echo "$i"|grep littlefs || echo "$i"|grep bmx280 || (
	  cd "$i" && {
		 git status|grep "On branch $branch" && {
		   git "$cmd" origin "$branch"
       [ "$do_github" = "1" -a $cmd" = 'push' ] && git "$cmd" github "$branch"	 
     }
	  }
	 ); 
  done
  git "$cmd" origin "$branch"
  [ "$do_github" = "1" -a "$cmd" = 'push' ] && git "$cmd" github "$branch"
 done
 
