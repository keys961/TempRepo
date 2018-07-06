<%--
  Created by IntelliJ IDEA.
  User: keys9
  Date: 2018/5/13 0013
  Time: 10:16
  To change this template use File | Settings | File Templates.
--%>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<html>
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <!-- Using CSS of Bootstrap -->
    <link href="/bootstrap.min.css" rel="stylesheet">
    <link href="/info.css" rel="stylesheet">
    <title></title>
</head>
<body>
    <div class="container">
    <!-- Navbar -->
    <div class="row clearfix">
        <div class="col-md-12 column">
            <nav class="navbar navbar-inverse navbar-fixed-top" role="navigation">
                <div class="navbar-header">
                    <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#bs-example-navbar-collapse-1">
                        <span class="sr-only">Toggle navigation</span><span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                    </button>
                    <a class="navbar-brand" href="<c:url value="/"/>">单词查询</a>
                </div>
            </nav>
            <div class="container-fluid">
                <div class="row-fluid">
                    <div class="span12">
                        <h2>
                            <br />
                        </h2>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>
    <div class="container">
        <div class="row clearfix">
            <div class="col-md-3 column">
            </div>
            <div class="col-md-6 column">
                    <div class="jumbotron">
                        <div>
                            <h1>${word.word}</h1>
                            <hr>
                            <br>
                            <p> <b>发音:</b> ${word.phonetic} </p>
                            <p> <b>释义:</b> <br> ${word.translation} </p>
                            <p> <b>类别标签</b>: ${word.tag} </p><br>
                        </div>
                    </div>
            </div>
            <div class="col-md-3 column">
            </div>
        </div>
    </div>
    <script src="https://cdn.bootcss.com/jquery/2.1.1/jquery.min.js"></script>
    <script src="https://cdn.bootcss.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
</body>
</html>
