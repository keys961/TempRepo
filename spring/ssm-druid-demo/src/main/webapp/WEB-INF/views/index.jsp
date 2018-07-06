<%@ taglib prefix="sp" uri="http://www.springframework.org/tags" %>
<%--
  Created by IntelliJ IDEA.
  User: keys9
  Date: 2018/5/9 0009
  Time: 9:13
  To change this template use File | Settings | File Templates.
--%>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <!-- Using CSS of Bootstrap -->
    <link href="/bootstrap.min.css" rel="stylesheet">
    <link href="/info.css" rel="stylesheet">
    <title>查询单词</title>
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
                    <h1>查询单词</h1>
                    <p>输入查询的单词:</p>
                    <form role="form">
                        <input id="word" type="text" class="form-control"
                               placeholder="输入查询的单词" name="word"/>
                        <br>
                    </form>
                    <button class="btn btn-primary" id="search">点击查询</button>
                    <hr>
                </div>
            </div>
            <div class="col-md-3 column">
            </div>
        </div>
    </div>
    <script src="https://cdn.bootcss.com/jquery/2.1.1/jquery.min.js"></script>
    <script src="https://cdn.bootcss.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
    <script type="application/javascript">
        $(document).ready
        (
            function ()
            {
                $("#search").click
                (
                    function ()
                    {
                        let word = $("#word").val();
                        $.ajax
                        (
                            {
                                type: "POST",
                                url: "<sp:url value="/search"/>",
                                data: word,
                                datatype: "json",
                                contentType: "application/json;charset=UTF-8",
                                success:
                                    function (result)
                                    {
                                        let id = result.id;
                                        window.open("/vocabulary/" + id, "_blank");
                                    },
                                error:
                                    function()
                                    {
                                        alert("未找到单词!");
                                    }

                            }
                        )
                    }
                )
            }
        );
    </script>
</body>
</html>
