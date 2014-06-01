(define (double x) (* x 2))

(create-elements!
  (column
    (row 
      (image
        (id 'img-1)
        (prop 'source "img/infantry_1.png")
        (prop 'width (bound '(this source-width)))
        (prop 'height (bound '(this source-height))))
      (image 
        (id 'img-2)
        (prop 'source "img/infantry_1.png")
        (prop 'width (bound '(img-1 width) double))
        (prop 'height (bound '(img-1 height) double)))
      (image 
        (id 'img-3)
        (prop 'source "img/infantry_1.png")
        (prop 'width (bound '(img-2 height) double))
        (prop 'height (bound '(img-2 height) double))))
    (row 
      (image
        (id 'img-1)
        (prop 'source "img/infantry_1.png")
        (prop 'width 32)
        (prop 'height 32))
      (image 
        (id 'img-2)
        (prop 'source "img/infantry_1.png")
        (prop 'width (bound '(img-1 width) double))
        (prop 'height (bound '(img-1 height) double)))
      (image 
        (id 'img-3)
        (prop 'source "img/infantry_1.png")
        (prop 'width (bound '(img-2 height) double))
        (prop 'height (bound '(img-2 height) double))))))
  