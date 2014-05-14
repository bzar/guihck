(create-elements!
  (rectangle '(id button x 300 y 100 width 200 height 200)
    (mouse-area '(width (bind (get-prop (parent) 'width)) 
                  height (bind (get-prop (parent) 'height))
                  color '(0 0 255)
                  on-mouse-down (set-prop! (find-element 'button) 'color '(0 0 255))
                  on-mouse-up (set-prop! (find-element 'button) 'color '(255 0 255))
                  on-mouse-move (set-prop! (find-element 'button) 'color '(255 255 0))
                  on-mouse-enter (set-prop! (find-element 'inner) 'color '(255 0 0))
                  on-mouse-exit (set-prop! (find-element 'inner) 'color '(0 255 0)))
      (rectangle '(id inner x 75 y 75 width 50 height 50)))))
    