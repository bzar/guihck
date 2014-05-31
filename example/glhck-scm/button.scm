(define (button-gen)
  (define center-x
    (bound '(this width parent width) (lambda (w pw) (/ (- pw w) 2))))
  (define center-y
    (bound '(this height parent height) (lambda (h ph) (/ (- ph h) 2))))

  (composite rectangle 
    (prop 'width 64)
    (prop 'height 32)
    (prop 'color '(240 240 240))
    (alias 'on-clicked 'mouse-area 'on-mouse-down)
    (text 
      (prop 'text "Button!")
      (prop 'x center-x)
      (prop 'y center-y)
      (prop 'color '(24 24 24)))
    (mouse-area 
      (id 'mouse-area)
      (prop 'width (bound '(parent width) identity))
      (prop 'height (bound '(parent height) identity))
      (method 'on-mouse-down (lambda (b x y)
        (set-prop! (parent) 'color '(255 255 255))))
      (method 'on-mouse-up (lambda (b x y)
        (set-prop! (parent) 'color '(240 240 240)))))))
(define button (button-gen))

(create-elements!
  (row 
    (prop 'x 100) 
    (prop 'y 100)
    (prop 'spacing 16)
    (column 
      (prop 'spacing 8)
      (button)
      (button 
        (prop 'width 128)))
    (column 
      (prop 'spacing 8)
      (button)
      (button 
        (prop 'height 64))
      (button 
        (method 'on-clicked (lambda (b x y) 
          (set-prop! 'color '(255 0 0))))))))