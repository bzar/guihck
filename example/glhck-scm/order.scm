(define orderable
  (composite rectangle
    (prop 'order 0)
    (prop 'block #t)
    (text
      (align 'center)
      (prop 'size (bound '(parent height) (lambda (h) (/ h 2))))
      (prop 'text (bound '(parent order) object->string)))
    (mouse-area
      (fill-parent)
      (method 'on-click (lambda (b x y) 
        (set-prop! (parent) 'order (+ 1 (get-prop (parent) 'order)))
        (get-prop (parent) 'block))))))
        
(create-elements!
  (orderable
    (prop 'color '(255 0 0))
    (align 'horizontal-center)
    (align 'vertical-center -30))
  (orderable
    (prop 'color '(0 255 0))
    (align 'horizontal-center -30)
    (align 'vertical-center 40))
  (orderable
    (prop 'block #f)
    (prop 'color '(0 0 255))
    (align 'horizontal-center 30)
    (align 'vertical-center 30)))
        