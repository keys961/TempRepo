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
                        url: "/search",
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