<template>
    <div class='parent'>
        <h1 style='color: var(--vue-green);'>{{ isLogin ? 'Login' : 'Sign Up' }}</h1>
        <div style="width: 90%;">
            <form @submit.prevent='handleSubmit'>
                <div class='form-data'>
                    <label for='username'>Username</label>
                    <input type='text' id='username' v-model='form_data.username'
                           placeholder='Enter your username' />

                    <label for='password'>Password</label>
                    <input type='password' id='password' v-model='form_data.password'
                           placeholder='Enter your password' @input='checkRequirements' @focus='show_password_requirements = !isLogin'
                        @blur='hideChecker'/>

                    <div v-if='show_password_requirements && !isLogin' class='password-requirements'>
                        <p>Password must contain:</p>
                        <ul>
                            <li v-for='(item, index) in password_requirements' :key='index'
                                :class="{'error': !item.isValid, 'success': item.isValid}">
                                {{ item.text }}
                            </li>
                        </ul>
                    </div>
                </div>


                <button type='submit' class='button-green submit'>{{ isLogin ? 'Login' : 'Sign Up' }}</button>
            </form>


            <p v-if='result.message' :class="{'error': result.is_error, 'success':
                                             !result.is_error}">{{ result.message }}</p>
        </div>

        <p style='margin-top: auto'>
        <span @click='toggleForm' class='link'>{{ isLogin? 'Don\'t have an account? Sign Up' : 'Already have an account? Login' }}</span>
        </p>
    </div>
</template>

<script setup>
    import { ref } from 'vue'
    import axios from 'axios'
    import { useRouter } from 'vue-router'
    import { login } from '@/auth'
    
    const router = useRouter()

    const isLogin = ref(true)
    const show_password_requirements = ref(false)

    const result = ref({
        message: '',
        is_error: false
    })

    const form_data = ref({
        username: '',
        password: ''
    })

    const password_requirements = ref([
        { text: 'At least 8 characters', isValid: false },
        { text: 'At least one uppercase character', isValid: false },
        { text: 'At least one lowercase character', isValid: false },
        { text: 'At least one number', isValid: false },
        { text: 'At least one special character', isValid: false }
    ])

// Function to validate password while typing
    const checkRequirements = () => {
        const password = form_data.value.password;

        password_requirements.value[0].isValid = password.length >= 8
        password_requirements.value[1].isValid = /[A-Z]/.test(password)
        password_requirements.value[2].isValid = /[a-z]/.test(password)
        password_requirements.value[3].isValid = /\d/.test(password)
        password_requirements.value[4].isValid = /[`~!@#$%^&*()_\-+=\/\\|,?<>;:'"\.]/.test(password)

        show_password_requirements.value = true;
    };

    const hideChecker = () => {
        if (!form_data.value.password) {
            show_password_requirements.value = false
        }
    }

    const toggleForm = () => {
        isLogin.value = !isLogin.value
        form_data.value.password = ''
        checkRequirements()
        show_password_requirements.value = false
        result.value = { message: '', is_error: false }
    }
    
    const validatePassword = (password) => {
        const password_regex = /^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[`~!@#$%^&*()_\-+=\/\\|,?<>;:'"\.]).{8,}$/
        return password_regex.test(password)
    }

    const handleSubmit = async () => {
        result.value = { message: '', is_error: false }

        if (!isLogin.value && !validatePassword(form_data.value.password)) {
            result.value = { message: 'Password does not meet requirements', is_error: true }
            return
        }

        try {
            const response = isLogin.value
                ? await axios.post(`${import.meta.env.VITE_REST_URL}/login`, form_data.value)
                : await axios.post(`${import.meta.env.VITE_REST_URL}/users`, form_data.value)

            if (response.status == 200 || response.status == 204) {
                result.value = {
                    message: isLogin.value
                        ? 'Login successful'
                        : 'Sign up successful, please login',
                    is_error: false
                }
                    
                if (isLogin.value) {
                    const token = response.data.token

                    if (token) {
                        login(token, router)    // Store the token and navigate to home
                    }
                    else {
                        result.value = { message: 'Token missing in response', is_error: true }
                    }
                }
            }
        }
        catch (error) {
            if (error.response) {
                result.value = {
                    message: error.response.data.error || 'An unexpected error occurred',
                    is_error: true
                }
            }
            else {
                result.value = { message: 'Failed to connect to the server', is_error: true }
            }
        }
    }
</script>


<style scoped>
.parent {
    display: flex;
    flex-direction: column;
    align-items: center;
    width: 30%;
    min-width: 320px;
    padding: 20px;
    border: 3px solid var(--vue-green);
    background-color: var(--foreground);
}

.form-data {
    display: flex;
    flex-direction: column;
    position: relative;
}

.form-data label {
    font-size: 1.5em;
    text-align: left;
    margin-top: 10px;
}

.form-data input {
    font-size: 1.25em;
    min-height: 40px;
    justify-content: center;
    align-items: center;
}

.form-data input:focus {
    outline: none;
}

.submit {
    width: 100%;
    font-size: 14pt;
    padding: 10px;
    margin-top: 20px;
    margin-bottom: 20px;
}

.link {
    font-size: 12pt;
    color: var(--vue-green);
    cursor: pointer;
    text-decoration: underline;
}

.link:hover {
    text-decoration: none;
}

.password-requirements {
    position: absolute;
    top: 70%;
    left: 120%;
    padding: 10px;
    border: 1px solid #ddd;
    background-color: var(--foreground);
    width: 300px;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    z-index: 10;
}

.password-requirements p {
    font-size: 14pt;
    font-weight: bold;
    margin-bottom: 5px;
}

.password-requirements li {
    text-align: left;
    font-size: 12pt;
}

</style>
