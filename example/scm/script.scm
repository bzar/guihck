(push-new-element! 'rectangle)
(set-element-property! 'x 200.0)
(set-element-property! 'y 100.0)
(set-element-property! 'width 50.0)
(set-element-property! 'height 75.0)
(set-element-property! 'color '(128 52 200))
(pop-element!)



(define (create-rectangle! x y w h c) 
  (begin 
    (push-new-element! 'rectangle)
    (set-element-property! 'x x)
    (set-element-property! 'y y)
    (set-element-property! 'width w)
    (set-element-property! 'height h)
    (set-element-property! 'color c)
    (pop-element!)))
   
(create-rectangle! 50 50 50 50 '(255 0 255))

(create-elements!
  (rectangle
    (prop 'x 400)
    (prop 'y 400)
    (prop 'width 25)
    (prop 'height 25)
    (rectangle
      (prop 'x 10)
      (prop 'y 10)
      (prop 'width 10)
      (prop 'height 10)
      (prop 'color '(255 0 0)))
    (rectangle
      (prop 'x 15)
      (prop 'y 15)
      (prop 'width 10)
      (prop 'height 10)
      (prop 'color '(0 255 0))
      (rectangle
        (prop 'width 5)
        (prop 'height 5))))
  (rectangle
    (prop 'x 700)
    (prop 'y 100)
    (prop 'width 25)
    (prop 'height 25)
    (prop 'color '(0 0 255))))
