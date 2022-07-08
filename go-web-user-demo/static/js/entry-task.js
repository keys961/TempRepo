$(document).ready(function () {
    $("#register-form").validate({
        rules: {
            username: {
                required: true,
                rangelength: [6, 64]
            },
            nickname: {
                required: true,
                rangelength: [6, 64]
            },
            password: {
                required: true,
                rangelength: [6, 64]
            },
        },
        messages: {
            username: {
                required: "Please input username",
                rangelength: "Length range from 6 to 64"
            },
            nickname: {
                required: "Please input nickname",
                rangelength: "Length range from 6 to 64"
            },
            password: {
                required: "Please input password",
                rangelength: "Length range from 6 to 64"
            },
        },
        submitHandler: function (form) {
            const urlStr = "/user/register";
            $(form).ajaxSubmit({
                url: urlStr,
                type: "POST",
                dataType: "json",
                success: function (data, status) {
                    if (data.status.code === 0) {
                        setTimeout(function () {
                            window.location.href = "/user/login"
                        }, 1000)
                    } else {
                        alert("err: " + data.status.msg)
                    }
                },
                err: function (data, status) {
                    alert("err: " + data.status.msg + ":" + status)
                }
            })
        }
    })

    $("#login-form").validate({
        rules: {
            username: {
                required: true,
                rangelength: [6, 64]
            },
            password: {
                required: true,
                rangelength: [6, 64]
            },
        },
        messages: {
            username: {
                required: "Please input username",
                rangelength: "Length range from 6 to 64"
            },
            password: {
                required: "Please input password",
                rangelength: "Length range from 6 to 64"
            },
        },
        submitHandler: function (form) {
            const urlStr = "/user/login";
            $(form).ajaxSubmit({
                url: urlStr,
                type: "POST",
                dataType: "json",
                success: function (data, status) {
                    if (data.status.code === 0) {
                        alert("login success, redirect to main page")
                        setTimeout(function () {
                            window.location.href = "/"
                        }, 1000)
                    } else {
                        alert("err: " + data.status.msg)
                    }
                },
                err: function (data, status) {
                    alert("err: " + data.status.msg + ":" + status)
                }
            })
        }
    })

    $("#profile-form").validate({
        rules: {
            username: {
                required: true,
                rangelength: [6, 64]
            },
            nickname: {
                required: true,
                rangelength: [6, 64]
            },
        },
        messages: {
            username: {
                required: "Please input username",
                rangelength: "Length range from 6 to 64"
            },
            nickname: {
                required: "Please input nickname",
                rangelength: "Length range from 6 to 64"
            },
        },
        submitHandler: function (form) {
            const urlStr = "/user/profile/update";
            $(form).ajaxSubmit({
                url: urlStr,
                type: "POST",
                dataType: "json",
                success: function (data, status) {
                    if (data.status.code === 0) {
                        alert("Update success.")
                        setTimeout(function () {
                            window.location.href = "/user/profile"
                        }, 1000)
                    } else {
                        alert("err: " + data.status.msg)
                    }
                },
                err: function (data, status) {
                    alert("err: " + data.status.msg + ":" + status)
                }
            })
        }
    })

    $("#avatar-upload-button").click(function () {
        var filedata = $("#avatar-upload-file").val();
        if (filedata.length <= 0) {
            alert("Please upload a file!");
            return
        }
        var data = new FormData()
        data.append("file", $("#avatar-upload-file")[0].files[0]);
        // alert(data)
        var urlStr = "/user/profile/avatar"
        $.ajax({
            url: urlStr,
            type: "POST",
            dataType: "json",
            contentType: false,
            data: data,
            processData: false,
            success: function (data, status) {
                if (data.status.code === 0) {
                    alert("Upload avatar success.")
                    setTimeout(function () {
                        window.location.href = "/user/profile"
                    }, 1000)
                } else {
                    alert("err: " + data.status.msg)
                }
            },
            err: function (data, status) {
                alert("err: " + data.status.msg + ":" + status)
            }
        })
    })
})

function logout() {
    var urlStr = "/user/logout"
    if (confirm("Confirm logout?")) {
        $.ajax({
            url: urlStr,
            type: "GET",
            success: function (data, status) {
                alert("Logout success.")
                setTimeout(function () {
                    window.location.href = "/"
                }, 1000)
            },
            err: function (data, status) {
                alert("err: " + data.status.msg + ":" + status)
            }
        })
    }
}