<template>
    <div class="container">
        <h1>Login</h1>
        <form @submit.prevent="handleSubmit" class="fields">
            <input type="text" id="username" placeholder="Username" v-model="formData.username">
            <input type="password" id="password" placeholder="Password" v-model="formData.password">
            <button type="submit">Sign up</button>
        </form>
        <p style="padding-left: 20px">Forgot password? Good luck!</p>
        <p>Don't have an account? <a style="color: var(--vue-green)">Sign up</a></p>
    </div>
</template>

<script>
    export default {
        data() {
            return {
                formData: {
                    username: '',
                    password: ''
                }
            };
        },
        methods: {
            handleSubmit() {
                console.log('Form data: ', this.formData);
            }
        }
    };
</script>

<style>
.container {
    width: 400px;
    height: 300px;
    position: absolute;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    background-color: var(--foreground);
    border: 4px solid var(--vue-green);
}

.fields {
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    margin-top: 20px;
    margin-bottom: 20px;
}

.fields input {
    width: 300px;
    height: 40px;
    font-size: 12pt;
    margin-bottom: 10px;
}

.fields button {
    width: 70px;
    height: 30px;
    font-size: 11pt;
    margin-top: 10px;
}

h1 {
    color: var(--vue-green);
    margin-top: 20px;
}

a {
}

</style>
