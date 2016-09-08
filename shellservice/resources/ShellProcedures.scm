(define (us-ls)
  (us-display-bundle-info (us-bundle-info "_header_"))
  (us-display-bundle-info
   (map (lambda (i) (us-bundle-info i)) (us-bundle-ids))))
