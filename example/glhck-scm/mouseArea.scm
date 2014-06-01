(create-elements!
  (rectangle 
    (id 'button) 
    (align 'center)
    (prop 'width 200)
    (prop 'height 200)
    
    (mouse-area 
      (fill-parent)
      (prop 'color '(0 0 255))
      (method 'on-mouse-down (lambda (b x y) 
        (set-prop! (find-element 'button) 'color '(0 0 255))))
      (method 'on-mouse-up (lambda (b x y) 
        (set-prop! (find-element 'button) 'color '(255 0 255))))
      (method 'on-mouse-move (lambda (sx sy dx dy) 
        (set-prop! (find-element 'button) 'color '(255 255 0))))
      (method 'on-mouse-enter (lambda (sx sy dx dy) 
        (set-prop! (find-element 'inner) 'color '(255 0 0))))
      (method 'on-mouse-exit (lambda (sx sy dx dy) 
          (set-prop! (find-element 'inner) 'color '(0 255 0))))
          
      (rectangle 
        (id 'inner)
        (align 'center)
        (prop 'width 50)
        (prop 'height 50)
        (prop 'color '(0 0 0))))))
    