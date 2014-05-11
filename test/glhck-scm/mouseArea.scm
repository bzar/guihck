(create-elements
  (rectangle '(id button x 300 y 100 width 200 height 200)
    (mouse-area '(x 0 y 0 width 200 height 200 
                  color '(0 0 255)
                  on-mouse-down (set-prop! (find-element! 'button) 'color '(0 0 255))
                  on-mouse-up (set-prop! (find-element! 'button) 'color '(255 0 255))
                  on-mouse-move (set-prop! (find-element! 'button) 'color '(255 0 0))
                  on-mouse-enter (set-prop! (find-element! 'button) 'color '(255 0 0))
                  on-mouse-exit (set-prop! (find-element! 'button) 'color '(0 255 0))
                  ))))
    