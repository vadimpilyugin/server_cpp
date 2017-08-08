#! /home/vadim/.rbenv/shims/ruby
s = <<DOC
<!DOCTYPE html>
<html>
	<head>
		<link href='/css/bootstrap.min.css' rel="stylesheet">
	    <link href='/css/style.css' rel="stylesheet">
	    <script src='/js/jquery.min.js'></script>
	    <script src='/js/bootstrap.min.js'></script>
	</head>
	<body>
		<div class="container">
			<div class="col-md-8">
				<h1> Hello, World! </h1>
				<button class="btn btn-primary"> To Wonderland </button>
DOC
s += "<div id=\"env\" class=\"\">"
ENV.each do |a,b|
	s += "<p> #{a} = #{b} </p>"
end
s += "</div>"
s += <<BOB
			</div>
		</div>
	</body>
</html>
BOB
puts s