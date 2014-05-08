(create-element! 'rectangle)
(set-element-property! 'x 200.0)
(set-element-property! 'y 100.0)
(set-element-property! 'width 50.0)
(set-element-property! 'height 75.0)
(set-element-property! 'color '(128 52 200))
(pop-element!)



(define (create-rectangle! x y w h c) 
  (begin 
    (create-element! 'rectangle)
    (set-element-property! 'x x)
    (set-element-property! 'y y)
    (set-element-property! 'width w)
    (set-element-property! 'height h)
    (set-element-property! 'color c)
    (pop-element!)))
   
(create-rectangle! 50 50 50 50 '(255 0 255))

(create-elements 
  (rectangle '(x 400
               y 400
               width 25
               height 25)
    (rectangle '(x 410
                 y 410
                 width 10
                 height 10
                 color (255 0 0)))
    (rectangle '(x 415
                 y 415
                 width 10
                 height 10
                 color (0 255 0))))
  (rectangle '(x 700
               y 100
               width 25
               height 25
               color (0 0 255))))