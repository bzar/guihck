(define color-button
  (composite rectangle
    (prop 'width 50)
    (prop 'height 50)
    (mouse-area
      (prop 'on-mouse-down (lambda (b x y)
        (set-clear-color (get-prop (parent) 'color))))
      (prop 'width 50)
      (prop 'height 50))))
      
(create-elements!
  (row 
    (prop 'x 100)
    (prop 'y 100)
    (color-button
      (prop 'color '(255 0 0)))
    (color-button
      (prop 'color '(0 255 0)))
    (color-button
      (prop 'color '(0 0 255)))))
                  
    
